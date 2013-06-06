#include <vector>
#include <cassert>

#include "xquery_ast_utils.h"

namespace xquery
{

void ContextIterator::ctx_iterator::IncSetIterator(size_t idx)
{
    auto& it = set_iter_[idx];

    ref_node_->ast_->CtxPopVarDef();
    if (++it == std::end(ctx_[idx].second)) {
        if (idx == 0)
            ended_ = true;
        else
            IncSetIterator(idx - 1);
    }
    else
        ref_node_->ast_->CtxPushVarDef(ctx_[idx].first, {*it});

    // Reevaluate the upper variable definitions
    if (!ended_ && idx < ctx_.size() - 1) {
        ++idx;
        // XXX: Here an empty `EvalResult' is tolerated (see xquery_nodes.cc)
        ref_node_->edges_[idx]->Eval({});
        auto vdef = ref_node_->ast_->CtxPopVarDef();
        auto it = std::begin(vdef.second);
        ref_node_->ast_->CtxPushVarDef(vdef.first, {*it});
        ctx_[idx] = std::move(vdef);
        set_iter_[idx] = std::move(it);
    }
}

ContextIterator::ctx_iterator ContextIterator::begin(const Node* node) const
{
    Ast::Context  ctx;
    NodeListSetIt set_iter;

    for (auto edge : node->edges_) {
        // XXX: Here an empty `EvalResult' is tolerated (see xquery_nodes.cc)
        edge->Eval({});
        auto vdef = node->ast_->CtxPopVarDef();
        auto it = std::begin(vdef.second);
        node->ast_->CtxPushVarDef(vdef.first, {*it});
        ctx.push_back(std::move(vdef));
        set_iter.push_back(std::move(it));
    }
    return {node, std::move(ctx), std::move(set_iter)};
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
        assert(true); // Copying context iterator invalidate its state
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
