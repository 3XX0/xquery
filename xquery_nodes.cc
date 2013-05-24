#include "xquery_nodes.h"
#include "xquery_xml.h"

#define FIRST 0
#define LEFT  0
#define RIGHT 1

namespace xquery { namespace lang
{

Node::EvalResult NonTerminalNode::Eval(const EvalResult& res) const
{
    return edges_[FIRST]->Eval(res);
}

Node::EvalResult TagName::Eval(const EvalResult& res) const
{
    xml::NodeList children, ret_nodes;

    for (auto node : res.nodes) {
        children = node->get_children(tagname_);
        ret_nodes.splice(std::end(ret_nodes), children);
    }
    return ret_nodes;
}

Node::EvalResult Text::Eval(const EvalResult& res) const {}

Node::EvalResult Document::Eval(const EvalResult& res) const
{
    xml::NodeList ret_node;

    //TODO Exception not caught
    //parser_.set_validate();
    parser_.parse_file(name_);
    assert(parser_);

    ret_node.push_back(parser_.get_document()->get_root_node());
    return ret_node;
}

Node::EvalResult PathSeparator::Eval(const EvalResult& res) const
{
    xml::NodeList ret_nodes;
    xml::NodeSet  children;

    const EvalResult left_res = edges_[LEFT]->Eval(res);

    if (sep_ == DESC_OR_SELF)
    {
        assert(left_res.type == EvalResult::NODES);
        for (auto node : left_res.nodes) {
            children = node->find("//*");
            ret_nodes.insert(std::end(ret_nodes), std::begin(children), std::end(children));
        }
        return edges_[RIGHT]->Eval(ret_nodes);
    }
    else
        return edges_[RIGHT]->Eval(left_res);
}

Node::EvalResult PathGlobbing::Eval(const EvalResult& res) const {}
Node::EvalResult Precedence::Eval(const EvalResult& res) const {}
Node::EvalResult Concatenation::Eval(const EvalResult& res) const {}
Node::EvalResult Filter::Eval(const EvalResult& res) const {}
Node::EvalResult Equality::Eval(const EvalResult& res) const {}
Node::EvalResult LogicOperator::Eval(const EvalResult& res) const {}
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
