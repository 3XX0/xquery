#include <vector>

#include "xquery_ast_utils.h"

namespace xquery
{

bool ContextIterator::ctx_iterator::IncSetIterator(size_t idx)
{
    auto it = set_iter_[idx];
    ++it;
    if (it == std::end(ctx_[idx].second)) {
        if (idx == 0)
            return false; // Do not reset
        if ( IncSetIterator(idx - 1))
            it = std::begin(ctx_[idx].second);
        else
            return false;
    }
    return true;
}

Node::EvalResult::EvalResult(EvalResult&& res) : type{res.type}
{
    if (type == NODES)
        new (&nodes) xml::NodeList{std::move(res.nodes)};
    else if (type == COND)
        condition = res.condition;
    else if (type == CTX_IT)
        new (&iterator) ContextIterator::ctx_iterator{std::move(res.iterator)};
}

Node::EvalResult& Node::EvalResult::operator=(EvalResult&& res)
{
    using NodeList = xml::NodeList;
    using ctx_iterator = ContextIterator::ctx_iterator;

    if (&res != this) {
        if (type == NODES)
            nodes.~NodeList();
        else if (type == CTX_IT)
            iterator.~ctx_iterator();
        if (res.type == NODES)
            new (&nodes) xml::NodeList{std::move(res.nodes)};
        else if (res.type == COND)
            condition = res.condition;
        else if (res.type == CTX_IT)
            new (&iterator) ContextIterator::ctx_iterator{std::move(res.iterator)};
        type = res.type;
    }
    return *this;
}

Node::EvalResult::EvalResult(const EvalResult& res) : type{res.type}
{
    if (type == NODES)
        new (&nodes) xml::NodeList{res.nodes};
    else if (type == CTX_IT)
        new (&iterator) ContextIterator::ctx_iterator{res.iterator};
    else if (type == COND)
        condition = res.condition;
}

Node::EvalResult::~EvalResult()
{
    using NodeList = xml::NodeList;
    using ctx_iterator = ContextIterator::ctx_iterator;

    if (type == NODES)
        nodes.~NodeList();
    else if (type == CTX_IT)
        iterator.~ctx_iterator();
}

}
