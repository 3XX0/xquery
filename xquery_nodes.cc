#include "xquery_nodes.h"

namespace xquery { namespace lang
{

Node::EvalResult NonTerminalNode::Eval(const EvalResult& res) const {}
Node::EvalResult TagName::Eval(const EvalResult& res) const {}
Node::EvalResult Text::Eval(const EvalResult& res) const {}
Node::EvalResult Document::Eval(const EvalResult& res) const {}
Node::EvalResult PathSeparator::Eval(const EvalResult& res) const {}
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
