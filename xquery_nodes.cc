#include <iostream>
#include "xquery_nodes.h"
#include "xquery_xml.h"

#define FIRST 0
#define LEFT  0
#define RIGHT 1

#define HAS_NODES(x) (x.type == EvalResult::NODES)
#define HAS_COND(x) (x.type == EvalResult::COND)

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

Node::EvalResult Document::Eval(const EvalResult& res) const
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
            children = node->find("//*");
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
        auto n1_elem= dynamic_cast<const xml::Element*>(n1);
        auto n2_elem = dynamic_cast<const xml::Element*>(n2);
        // Both are elements
        if (n1_elem && n2_elem) {
            auto n1_text = n1_elem->get_child_text();
            auto n2_text = n2_elem->get_child_text();
            // Either the same text or none
            if ((n1_text && n2_text &&
                    n1_text->get_content() == n2_text->get_content())
                    || (!n1_text && !n2_text)) {
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
        }
        else if (!n1_elem && !n2_elem)
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
    else if (eq_ == REF)
        return std::all_of(std::begin(left_res.nodes), std::end(left_res.nodes),
          [&it](const xml::Node* node){ return node == *it++; });
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
    else if (op_ == NOT) {
        auto first_res = edges_[FIRST]->Eval(res);
        return HAS_NODES(first_res) ? first_res.nodes.empty() : !first_res.condition;
    }
}

Node::EvalResult Variable::Eval(const EvalResult& res) const {}
Node::EvalResult ConstantString::Eval(const EvalResult& res) const {}
Node::EvalResult Tag::Eval(const EvalResult& res) const {}
Node::EvalResult LetClause::Eval(const EvalResult& res) const {}
Node::EvalResult WhereClause::Eval(const EvalResult& res) const {}
Node::EvalResult ForClause::Eval(const EvalResult& res) const {}
Node::EvalResult ReturnClause::Eval(const EvalResult& res) const {}
Node::EvalResult FLWRExpression::Eval(const EvalResult& res) const {}
Node::EvalResult LetExpression::Eval(const EvalResult& res) const {}
Node::EvalResult Tuple::Eval(const EvalResult& res) const {}
Node::EvalResult SomeClause::Eval(const EvalResult& res) const {}
Node::EvalResult Empty::Eval(const EvalResult& res) const {}

}}
