#include "xquery_nodes.h"
#include "xquery_xml.h"

#define FIRST 0
#define LEFT  0
#define RIGHT 1

namespace xquery { namespace lang
{

Node::EvalResult NonTerminalNode::Eval(EvalResult&& res) const
{
    return edges_[FIRST]->Eval(std::move(res));
}

Node::EvalResult TagName::Eval(EvalResult&& res) const
{
    xml::NodeList children, ret_nodes;

    assert(res.type == EvalResult::NODES);
    for (auto node : res.nodes) {
        children = node->get_children(tagname_);
        ret_nodes.splice(std::end(ret_nodes), children);
    }
    return ret_nodes;
}

Node::EvalResult Text::Eval(EvalResult&& res) const
{
    xml::NodeList ret_nodes;
    xml::Element* elem;

    assert(res.type == EvalResult::NODES);
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

Node::EvalResult Document::Eval(EvalResult&& res) const
{
    xml::NodeList ret_node;

    //TODO Exception not caught
    //parser_.set_validate();
    parser_.parse_file(name_);
    assert(parser_);

    ret_node.push_back(parser_.get_document()->get_root_node());
    return ret_node;
}

Node::EvalResult PathSeparator::Eval(EvalResult&& res) const
{
    xml::NodeList ret_nodes;
    xml::NodeSet  children;

    auto left_res = edges_[LEFT]->Eval(std::move(res));
    assert(left_res.type == EvalResult::NODES);

    // TODO unique filtering
    if (sep_ == DESC_OR_SELF) {
        for (auto node : left_res.nodes) {
            children = node->find("//*");
            ret_nodes.insert(std::end(ret_nodes), std::begin(children), std::end(children));
        }
        return edges_[RIGHT]->Eval(ret_nodes);
    }
    else if (sep_ == DESC)
        return edges_[RIGHT]->Eval(std::move(left_res));
}

Node::EvalResult PathGlobbing::Eval(EvalResult&& res) const
{
    xml::NodeList children, ret_nodes;

    assert(res.type == EvalResult::NODES);
    if (glob_ == SELF)
        return std::move(res);
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

Node::EvalResult Precedence::Eval(EvalResult&& res) const
{
    return edges_[FIRST]->Eval(std::move(res));
}

Node::EvalResult Concatenation::Eval(EvalResult&& res) const
{
    xml::NodeList ret_nodes;

    auto left_res = edges_[LEFT]->Eval(std::move(res));
    auto right_res = edges_[RIGHT]->Eval(std::move(res));
    assert(left_res.type == EvalResult::NODES);
    assert(right_res.type == EvalResult::NODES);

    ret_nodes.splice(std::end(ret_nodes), left_res.nodes);
    ret_nodes.splice(std::end(ret_nodes), right_res.nodes);
    return ret_nodes;
}

Node::EvalResult Filter::Eval(EvalResult&& res) const {}
Node::EvalResult Equality::Eval(EvalResult&& res) const {}
Node::EvalResult LogicOperator::Eval(EvalResult&& res) const {}

Node::EvalResult Variable::Eval(EvalResult&& res) const {}
Node::EvalResult ConstantString::Eval(EvalResult&& res) const {}
Node::EvalResult Tag::Eval(EvalResult&& res) const {}
Node::EvalResult LetClause::Eval(EvalResult&& res) const {}
Node::EvalResult WhereClause::Eval(EvalResult&& res) const {}
Node::EvalResult ForClause::Eval(EvalResult&& res) const {}
Node::EvalResult ReturnClause::Eval(EvalResult&& res) const {}
Node::EvalResult FLWRExpression::Eval(EvalResult&& res) const {}
Node::EvalResult LetExpression::Eval(EvalResult&& res) const {}
Node::EvalResult Tuple::Eval(EvalResult&& res) const {}
Node::EvalResult SomeClause::Eval(EvalResult&& res) const {}
Node::EvalResult Empty::Eval(EvalResult&& res) const {}

}}
