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
        using VarDefSet = std::unordered_map<std::string, xml::Node*>;

    public:
        class ctx_iterator
        {
            public:
                ctx_iterator(const Ast::Context& ctx, NodeListSetIt&& set_iter)
                  : ctx_{ctx},
                    set_iter_{set_iter} {}
                ctx_iterator(ctx_iterator&& it)
                  : ctx_{std::move(it.ctx_)},
                    set_iter_{std::move(it.set_iter_)} {}
                ctx_iterator(const ctx_iterator& it)
                  : ctx_{it.ctx_},
                    set_iter_{it.set_iter_} {}
                ~ctx_iterator() = default;

                ctx_iterator& operator++()
                {
                    IncSetIterator(set_iter_.size() - 1);
                    return *this;
                }
                bool operator==(const ctx_iterator& it) const
                {
                    return std::equal(std::begin(set_iter_), std::end(set_iter_),
                            std::begin(it.set_iter_));
                }
                bool operator!=(const ctx_iterator& it) const
                {
                    return !operator==(it);
                }
                VarDefSet operator*() const
                {
                    VarDefSet set;

                    for (size_t i = 0; i < ctx_.size(); ++i)
                        set[ctx_[i].first] = *set_iter_[i];
                    return set;
                }

            private:
                bool IncSetIterator(size_t idx);

                const Ast::Context& ctx_;
                NodeListSetIt       set_iter_;
        };

        ctx_iterator begin(const Ast::Context& ctx) const
        {
            NodeListSetIt set_iter;

            std::for_each(std::begin(ctx), std::end(ctx),
                    [&set_iter](const Ast::VarDef& vdef){ set_iter.push_back(std::begin(vdef.second)); });
            return {ctx, std::move(set_iter)};
        }
        ctx_iterator end(const Ast::Context& ctx) const
        {
            NodeListSetIt set_iter;

            std::for_each(std::begin(ctx), std::end(ctx),
                    [&set_iter](const Ast::VarDef& vdef){ set_iter.push_back(std::end(vdef.second)); });
            return {ctx, std::move(set_iter)};
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
