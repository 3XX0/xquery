#pragma once

#include <unordered_map>

#include "xquery_xml.h"
#include "xquery_ast.h"

namespace xquery
{

class ContextIterator
{
    protected:
        using NodeListIt = xml::NodeList::const_iterator;
        using NodeListSetIt = std::vector<NodeListIt>;

    public:
        class ctx_iterator
        {
            public:
                ctx_iterator(const Node* ref_node, Ast::Context&& ctx, NodeListSetIt&& set_iter)
                  : ref_node_{ref_node},
                    ctx_{std::move(ctx)},
                    set_iter_{std::move(set_iter)} {}
                ctx_iterator(bool ended) : ended_{ended} {}
                ctx_iterator(ctx_iterator&& it)
                  : ref_node_{it.ref_node_},
                    ctx_{std::move(it.ctx_)},
                    set_iter_{std::move(it.set_iter_)},
                    ended_{it.ended_} {}
                ~ctx_iterator() = default;

                ctx_iterator& operator++()
                {
                    IncSetIterator(set_iter_.size() - 1);
                    return *this;
                }
                bool operator==(const ctx_iterator& it) const
                {
                    return ended_ == it.ended_;
                }
                bool operator!=(const ctx_iterator& it) const
                {
                    return !operator==(it);
                }

            private:
                void IncSetIterator(size_t idx);

                const Node*   ref_node_ = nullptr;
                Ast::Context  ctx_;
                NodeListSetIt set_iter_;
                bool          ended_ = false;
        };

        ctx_iterator begin(const Node* node) const;
        ctx_iterator end() const
        {
            return true;
        }
};

struct Node::EvalResult
{
    enum UnionType
    {
        NODES,
        COND,
        CTX_IT,
        NONE
    };

    EvalResult() : type{NONE} {};
    EvalResult(xml::NodeList nl) : nodes{std::move(nl)}, type{NODES} {}
    EvalResult(bool cond) : condition{cond}, type{COND} {}
    EvalResult(ContextIterator::ctx_iterator it) : iterator{std::move(it)}, type{CTX_IT} {}
    EvalResult(EvalResult&& res);
    EvalResult& operator=(EvalResult&& res);
    EvalResult(const EvalResult& res);
    ~EvalResult();

    union {
        xml::NodeList                 nodes;
        bool                          condition;
        ContextIterator::ctx_iterator iterator;
    };
    UnionType type;
};

}
