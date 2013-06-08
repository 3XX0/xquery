#include "xquery_nodes.h"
#include "xquery_xml.h"

#define FIRST 0
#define LEFT  0
#define FOR   0

#define RIGHT 1
#define LET   1

#define WHERE 2
#define RET   3

#define HAS_NODES(x) (x.type == EvalResult::NODES)
#define HAS_COND(x) (x.type == EvalResult::COND)
#define HAS_CTX_IT(x) (x.type == EvalResult::CTX_IT)

#define NEW_NODE(...) ast_->AddNode(new __VA_ARGS__)
#define DEL_NODE(x) ast_->DeleteNode(x)

namespace xquery { namespace lang
{

Node::EvalResult NonTerminalNode::Eval(const EvalResult& res) const
{
    return edges_[FIRST]->Eval(res);
}

Node::EvalResult TagName::Eval(const EvalResult& res) const
{
    xml::NodeList children, ret_nodes;

    assert(HAS_NODES(res));
    for (auto node : res.nodes) {
        children = node->get_children(tagname_);
        ret_nodes.splice(std::end(ret_nodes), children);
    }
    return ret_nodes;
}

Node::EvalResult Text::Eval(const EvalResult& res) const
{
    xml::NodeList ret_nodes;
    xml::Element* elem;

    assert(HAS_NODES(res));
    for (auto node : res.nodes) {
        elem = dynamic_cast<xml::Element*>(node);
        if (elem == nullptr)
            throw std::runtime_error(node->get_name() +
              " is not a valid text node");
        else {
            auto n = static_cast<xml::Node*>(elem->get_child_text());
            if (n) ret_nodes.push_back(n);
        }
    }
    return ret_nodes;
}

Node::EvalResult Document::Eval(const EvalResult&) const
{
    //parser_.set_validate();
    parser_.parse_file(name_);
    assert(parser_);

    return xml::NodeList{parser_.get_document()->get_root_node()};
}

Node::EvalResult PathSeparator::Eval(const EvalResult& res) const
{
    xml::NodeList desc_nodes;
    xml::NodeSet  children;
    EvalResult    ret_res;

    auto left_res = edges_[LEFT]->Eval(res);
    assert(HAS_NODES(left_res));

    if (sep_ == DESC_OR_SELF) {
        for (auto node : left_res.nodes) {
            desc_nodes.push_back(node);    // Self
            children = node->find(".//*"); // Descendants
            desc_nodes.insert(std::end(desc_nodes),
              std::begin(children), std::end(children));
        }
        ret_res = edges_[RIGHT]->Eval(desc_nodes);
    }
    else if (sep_ == DESC)
        ret_res = edges_[RIGHT]->Eval(left_res);

    assert(HAS_NODES(ret_res));
    std::unique(std::begin(ret_res.nodes), std::end(ret_res.nodes));
    return ret_res;
}

Node::EvalResult PathGlobbing::Eval(const EvalResult& res) const
{
    xml::NodeList children, ret_nodes;

    assert(HAS_NODES(res));
    if (glob_ == SELF)
        return res;
    else if (glob_ == WILDCARD)
        for (auto node : res.nodes) {
            children = node->get_children();
            ret_nodes.splice(std::end(ret_nodes), children);
        }
    else if (glob_ == PARENT)
        for (auto node : res.nodes)
            ret_nodes.push_back(node->get_parent());
    return ret_nodes;
}

Node::EvalResult Precedence::Eval(const EvalResult& res) const
{
    return edges_[FIRST]->Eval(res);
}

Node::EvalResult Concatenation::Eval(const EvalResult& res) const
{
    xml::NodeList ret_nodes;

    auto left_res = edges_[LEFT]->Eval(res);
    auto right_res = edges_[RIGHT]->Eval(res);
    assert(HAS_NODES(left_res));
    assert(HAS_NODES(right_res));

    ret_nodes.splice(std::end(ret_nodes), left_res.nodes);
    ret_nodes.splice(std::end(ret_nodes), right_res.nodes);
    return ret_nodes;
}

Node::EvalResult Filter::Eval(const EvalResult& res) const
{
    xml::NodeList ret_nodes;

    auto left_res = edges_[LEFT]->Eval(res);
    assert(HAS_NODES(left_res));

    for (auto node : left_res.nodes) {
        auto right_res = edges_[RIGHT]->Eval(xml::NodeList{node});

        // Filter is a predicate
        if (HAS_COND(right_res) && right_res.condition == true)
            ret_nodes.push_back(node);
        // Filter is a RP
        else if (HAS_NODES(right_res))
            if ( !right_res.nodes.empty())
                ret_nodes.push_back(node);
    }
    return ret_nodes;
}

bool Equality::HasValueEquality(const xml::Node* n1, const xml::Node* n2) const
{
    // Same name
    if (n1->get_name() == n2->get_name()) {
        auto n1_elem = dynamic_cast<const xml::Element*>(n1);
        auto n2_elem = dynamic_cast<const xml::Element*>(n2);
        auto n1_text = dynamic_cast<const xml::TextNode*>(n1);
        auto n2_text = dynamic_cast<const xml::TextNode*>(n2);
        // Both are elements
        if (n1_elem && n2_elem) {
                auto n1_children = n1_elem->get_children();
                auto n2_children = n2_elem->get_children();
                auto it = std::begin(n2_children);
                // Same size
                if (n1_children.size() != n2_children.size())
                      return false;
                // All children respect the previous conditions
                return std::all_of(std::begin(n1_children), std::end(n1_children),
                  [this, &it](const xml::Node* node){ return HasValueEquality(node, *it++); });
        }
        // Both are text nodes
        else if (n1_text && n2_text)
            return n1_text->get_content() == n2_text->get_content();
        else if (!n1_elem && !n2_elem && !n1_text && !n2_text)
            return true;
    }
    return false;
}

Node::EvalResult Equality::Eval(const EvalResult& res) const
{
    auto left_res = edges_[LEFT]->Eval(res);
    auto right_res = edges_[RIGHT]->Eval(res);
    auto it = std::begin(right_res.nodes);

    assert(HAS_NODES(left_res));
    assert(HAS_NODES(right_res));
    if (left_res.nodes.empty() || right_res.nodes.empty() ||
        (left_res.nodes.size() != right_res.nodes.size()))
        return false;

    if (eq_ == VALUE)
        return std::all_of(std::begin(left_res.nodes), std::end(left_res.nodes),
          [this, &it](const xml::Node* node){ return HasValueEquality(node, *it++); });
    else // REF
        return std::all_of(std::begin(left_res.nodes), std::end(left_res.nodes),
          [&it](const xml::Node* node){ return node == *it++; });
}

void Equality::FindAll(Node* node, EqualityList& list)
{
    auto equality = dynamic_cast<Equality*>(node);
    if (equality != nullptr)
        list.push_back(equality);
    for (const auto& edge : node->edges())
        if (edge)
            FindAll(edge, list);
}

Node::EvalResult LogicOperator::Eval(const EvalResult& res) const
{
    xml::NodeList logic_set;

    if (op_ == AND || op_ == OR) {
        auto left_res = edges_[LEFT]->Eval(res);
        auto right_res = edges_[RIGHT]->Eval(res);

        if (HAS_COND(left_res) && HAS_COND(right_res)) // Both predicate
            return (op_ == AND) ? (left_res.condition && right_res.condition)
                                : (left_res.condition || right_res.condition);
        else if (HAS_NODES(left_res) && HAS_NODES(right_res)) { // Both RP
            auto it = std::begin(logic_set);
            logic_set.resize(left_res.nodes.size() + right_res.nodes.size());

            if (op_ == AND)
                it = std::set_intersection(std::begin(left_res.nodes), std::end(left_res.nodes),
                                           std::begin(right_res.nodes), std::end(right_res.nodes),
                                           std::begin(logic_set));
            else if (op_ == OR)
                it = std::set_union(std::begin(left_res.nodes), std::end(left_res.nodes),
                                    std::begin(right_res.nodes), std::end(right_res.nodes),
                                    std::begin(logic_set));

            logic_set.resize(std::distance(it, std::begin(logic_set)));
            return !logic_set.empty();
        }
        else if (HAS_COND(left_res) && HAS_NODES(right_res)) // Predicate and RP
            return (op_ == AND) ? (left_res.condition && !right_res.nodes.empty())
                                : (left_res.condition || !right_res.nodes.empty());
        else if (HAS_NODES(left_res) && HAS_COND(right_res)) // RP and predicate
            return (op_ == AND) ? (!left_res.nodes.empty() && right_res.condition)
                                : (!left_res.nodes.empty() || right_res.condition);
    }
    else { // NOT
        auto first_res = edges_[FIRST]->Eval(res);
        return HAS_NODES(first_res) ? first_res.nodes.empty() : !first_res.condition;
    }
    return {}; // Should not return here
}

/*
 * For XQuery, the `EvalResult' is actually not required.
 * std::optional C++14 ?
 */

Node::EvalResult Variable::Eval(const EvalResult&) const
{
    return ast_->CtxFindVarDef(varname_);
}

void Variable::FindAll(Node* node, VariableList& list)
{
    auto variable = dynamic_cast<Variable*>(node);
    if (variable != nullptr)
        list.push_back(variable);
    for (const auto& edge : node->edges())
        if (edge)
            FindAll(edge, list);
}

Node::EvalResult ConstantString::Eval(const EvalResult&) const
{
    xml::TextNode* cstring = ast_->CollectTextNode(cstring_);

    return xml::NodeList{cstring};
}

Node::EvalResult Tag::Eval(const EvalResult& res) const
{
    xml::Node* tag = ast_->CollectElement(tagname_);

    auto first_res = edges_[FIRST]->Eval(res);
    assert(HAS_NODES(first_res));
    for (auto node : first_res.nodes)
        tag->import_node(node);
    return xml::NodeList{tag};
}

Node::EvalResult LetClause::Eval(const EvalResult& res) const
{
    for (auto edge : edges_)
        edge->Eval(res);
    return {};
}

Node::EvalResult WhereClause::Eval(const EvalResult& res) const
{
    return edges_[FIRST]->Eval(res);
}

WhereClause::ImplicitJoins WhereClause::LookupImplicitJoins()
{
    // TODO find ALL the implicit join operations

    Equality::EqualityList  eq_list;
    ImplicitJoins           joins;

    Equality::FindAll(edges_[FIRST], eq_list);

    for (auto eq : eq_list) {
        const auto& edges = eq->edges();
        const auto& left_edges = edges[LEFT]->edges(); // XXX: LH XQ
        const auto& right_edges = edges[RIGHT]->edges(); // XXX: RHS XQ
        auto left_var = dynamic_cast<Variable*>(left_edges[FIRST]);
        auto right_var = dynamic_cast<Variable*>(right_edges[FIRST]);

        if (left_var && right_var) { // Found join operation
            joins.emplace_back(left_var->varname(), right_var->varname());
            auto child = static_cast<Node*>(eq);
            auto parent = child->parent();
            while (parent->edges().size() == 1 && parent != this) {
                child = parent;
                parent = parent->parent();
            }
            auto grandparent = parent->parent();

            if (parent == this) { // Our parent is a where clause
                grandparent->edges()[WHERE] = nullptr;
                DEL_NODE(this);
            }
            else { // Relink properly
                grandparent->DeleteEdge(parent);
                for (auto edge : parent->edges())
                    if (edge != child) {
                        parent->DeleteEdge(edge);
                        grandparent->AddEdge(edge);
                    }
                DEL_NODE(parent);
            }
            break;
        }
    }
    return joins;
}

Node::EvalResult ForClause::Eval(const EvalResult&) const
{
    return ctx_begin();
}

ForClause::VarDependencies ForClause::LookupDependencies() const
{
    // TODO build an inverted index to check for useless joins

    std::map<std::string, VariableDef*> vdef_names;
    VariableDef*                        vdef;
    VarDependencies                     var_dep;

    for (auto edge : edges_) {
        vdef = static_cast<VariableDef*>(edge);
        vdef_names.emplace(vdef->varname(), vdef);
    }
    for (auto edge : edges_) {
        vdef = static_cast<VariableDef*>(edge);
        auto var_ref = vdef->LookupReferences();

        for (auto ref : var_ref) {
            auto it = vdef_names.find(ref);
            if (it != std::end(vdef_names))
                var_dep[vdef->varname()].insert(it->second);
        }
        var_dep[vdef->varname()].insert(vdef);
    }
    return var_dep;
}

Node::EvalResult ReturnClause::Eval(const EvalResult& res) const
{
    return edges_[FIRST]->Eval(res);
}

Node::EvalResult FLWRExpression::Eval(const EvalResult& res) const
{
    xml::NodeList ret_nodes;
    EvalResult    ret_res;

    ast_->CtxNew();

    auto for_clause = static_cast<const ForClause*>(edges_[FOR]);
    auto for_res = for_clause->Eval(res);
    assert(HAS_CTX_IT(for_res));

    for (;for_res.iterator != for_clause->ctx_end(); ++for_res.iterator) {
        if (edges_[LET] != nullptr)
            edges_[LET]->Eval(res);
        if (edges_[WHERE] != nullptr) {
            auto where_res = edges_[WHERE]->Eval(res);
            assert(HAS_COND(where_res));
            if (where_res.condition == false)
                continue;
        }
        ret_res = edges_[RET]->Eval(res);
        assert(HAS_NODES(ret_res));
        ret_nodes.splice(std::end(ret_nodes), ret_res.nodes);
    }

    ast_->CtxDestroy();
    return ret_nodes;
}

Node* FLWRExpression::WriteNewExpression(const std::string& join_var,
                      const ForClause::VarDependencies& deps,
                      Node* where_clause) const
{
    Node::Edges for_edges;
    Node::Edges tuple_edges;

    // TODO recursive search
    for (const auto& dep : deps.at(join_var)) {
        dep->parent()->DeleteEdge(dep); // XXX: restrict variable sharing
        auto dep_name = dep->varname();
        for_edges.push_back(dep);
        auto var = NEW_NODE(Variable{dep_name});
        auto var_tag = NEW_NODE(Tag{dep_name, dep_name, {var}});
        tuple_edges.push_back(var_tag);
    }
    auto for_clause = NEW_NODE(ForClause{std::move(for_edges)});
    auto tuple_tag = NEW_NODE(Tag{"tuple", "tuple", std::move(tuple_edges)});
    auto ret_clause = NEW_NODE(ReturnClause{{tuple_tag}});

    return NEW_NODE(FLWRExpression{{for_clause, where_clause, nullptr, ret_clause}});
}

bool FLWRExpression::CorrelateDependencies(const std::string& join_var,
                     const ForClause::VarDependencies& deps,
                     const Variable::VariableList& var_list)
{
    const auto& dep_set = deps.at(join_var);

    for (auto var : var_list) {
        auto it = std::find_if(std::begin(dep_set), std::end(dep_set),
          [var](const VariableDef* vdef) { return vdef->varname() == var->varname(); });
        if (it == std::end(dep_set))
            return false;
    }
    return true;
}

void FLWRExpression::Rewrite()
{
    Variable::VariableList var_list;
    Node *where1 = nullptr, *where2 = nullptr;
    auto for_clause = static_cast<ForClause*>(edges_[FOR]);

    if (edges_[WHERE] != nullptr) {
        auto where_clause = static_cast<WhereClause*>(edges_[WHERE]);
        auto joins = where_clause->LookupImplicitJoins();
        if (!joins.empty()) { // A join is present, rewritting is needed
            auto deps = for_clause->LookupDependencies();
            // Where migration
            if (edges_[WHERE] != nullptr) {
                Variable::FindAll(edges_[WHERE], var_list);
                auto cor1 = CorrelateDependencies(joins[0].first, deps, var_list);
                auto cor2 = CorrelateDependencies(joins[0].second, deps, var_list);
                if (cor1 && !cor2)
                    where1 = edges_[WHERE];
                else if (!cor1 && cor2)
                    where2 = edges_[WHERE];
                else if (cor1 && cor2) // TODO We need to be smarter here
                    throw std::runtime_error("Could not resolve implicit join dependencies");
                if (cor1 || cor2)
                    edges_[WHERE] = nullptr;
            }
            // Join generation
            auto flwr1 = WriteNewExpression(joins[0].first, deps, where1);
            auto flwr2 = WriteNewExpression(joins[0].second, deps, where2);
            auto join = NEW_NODE(Join{{flwr1, flwr2}});
            auto vdef = NEW_NODE(VariableDef{"tuple", {join}});
            for_clause->AddEdge(vdef);
            // Return rewritting
            var_list.clear();
            Variable::FindAll(edges_[RET], var_list);
            for (auto var : var_list) {
                auto parent = var->parent();
                parent->DeleteEdge(var);
                auto tuple_var = NEW_NODE(Variable{"tuple"});
                auto tagname = NEW_NODE(TagName{var->varname()});
                auto psep = NEW_NODE(PathSeparator{"/", {tuple_var, tagname}});
                parent->AddEdge(psep);
                DEL_NODE(var);
            }
        }
    }
    Node::Rewrite();
}

Node::EvalResult LetExpression::Eval(const EvalResult& res) const
{
    ast_->CtxNew();
    edges_[LEFT]->Eval(res);
    auto ret_res = edges_[RIGHT]->Eval(res);
    ast_->CtxDestroy();
    return ret_res;
}

Node::EvalResult VariableDef::Eval(const EvalResult& res) const
{
    auto first_res = edges_[FIRST]->Eval(res);
    assert(HAS_NODES(first_res));
    ast_->CtxPushVarDef(varname_, std::move(first_res.nodes));
    return {};
}

VariableDef::VarReferences VariableDef::LookupReferences() const
{
    VarReferences           var_ref;
    Variable::VariableList  var_list;

    Variable::FindAll(edges_[FIRST], var_list);
    for (auto var : var_list)
        var_ref.insert(var->varname());
    return var_ref;
}

Node::EvalResult SomeExpression::Eval(const EvalResult& res) const
{
    ast_->CtxNew();

    auto some_clause = static_cast<const SomeClause*>(edges_[LEFT]);
    auto some_res = some_clause->Eval(res);
    assert(HAS_CTX_IT(some_res));

    for (;some_res.iterator != some_clause->ctx_end(); ++some_res.iterator) {
        auto sat_res = edges_[RIGHT]->Eval(res);
        assert(HAS_COND(sat_res));
        if (sat_res.condition == true) {
            ast_->CtxDestroy();
            return true;
        }
    }
    ast_->CtxDestroy();
    return false;
}

Node::EvalResult SomeClause::Eval(const EvalResult&) const
{
    return ctx_begin();
}

Node::EvalResult Empty::Eval(const EvalResult& res) const
{
    auto first_res = edges_[FIRST]->Eval(res);
    assert(HAS_NODES(first_res));
    return first_res.nodes.empty();
}

Node::EvalResult Join::Eval(const EvalResult& res) const
{

}

}}
