#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

#include "xquery_xml.h"
#include "xquery_misc.h"

namespace xquery
{

class Ast;

class Node : public NonCopyable, public NonMoveable
{
    friend class Ast;

    public:
        using Edges = std::vector<const Node*>;
        using const_iterator = Edges::const_iterator;

        struct EvalResult;

        virtual ~Node() = default;

        // Throws `std::runtime_error' or `xml::validity_error'
        virtual EvalResult Eval(const EvalResult& res) const = 0;

        const_iterator begin() const
        {
            return std::begin(edges_);
        }
        const_iterator end() const
        {
            return std::end(edges_);
        }
        void AddEdge(const Node* node) const
        {
            edges_.push_back(node);
        }
        const std::string& label() const
        {
            return label_;
        }

    protected:
        Node() = default;
        Node(Edges&& edges) : edges_{std::move(edges)}, ast_{nullptr} {}

        void set_label(const std::string& label)
        {
            label_ = label;
        }

        mutable Edges edges_;
        std::string   label_;
        size_t        id_ = 0;
        Ast*          ast_;

    private:
        /*
         * Ast specific
         */
        void set_ast(Ast* ast)
        {
            ast_ = ast;
        }
        void set_id(size_t id)
        {
            id_ = id;
        }
        size_t id() const
        {
            return id_;
        }
};

class Ast : public NonCopyable, public NonMoveable
{
    friend class Parser;

    using NodeUPtr = std::unique_ptr<const Node>;
    #define SCOPE_DELIM "{SD}"

    public:
        using VarDef = std::pair<std::string, xml::NodeList>;
        using Context = std::vector<VarDef>;

        Ast() = default;
        ~Ast() = default;

        void PlotGraph() const; // Throws `std::ios_base'
        void Evaluate() const;  // Throws `std::runtime_error'

        /*
         * Node specific
         */
        void CtxMarkScope()
        {
            context_stack_.emplace_front(SCOPE_DELIM, xml::NodeList{});
        }
        Context CtxUnmarkScope()
        {
            Context ctx;

            auto it = std::find_if(std::begin(context_stack_), std::end(context_stack_),
              [this](const VarDef& def) { return def.first == SCOPE_DELIM; });
            //ctx.insert(std::begin(ctx), std::begin(context_stack_), it);
            ctx.resize(context_stack_.size()); 
            std::move(std::begin(context_stack_), it, std::begin(ctx));
            ctx.resize(std::distance(std::begin(context_stack_), it));
            context_stack_.erase(std::begin(context_stack_), ++it);
            return ctx;
        }
        void CtxAddVarDef(const std::string& varname, xml::NodeList&& nodes)
        {
            context_stack_.emplace_front(varname, std::move(nodes));
        }
        const xml::NodeList& CtxFindVarDef(const std::string& varname)
        {
            // TODO Check boundaries and variable validity
            auto it = std::find_if(std::begin(context_stack_), std::end(context_stack_),
              [this, &varname](const VarDef& def) { return def.first == varname; });
            return it->second;
        }

    private:
        /*
         * Parser specific
         */
        const Node* AddNode(Node* node)
        {
            node->set_id(nodes_.size());
            node->set_ast(this);
            nodes_.push_back(NodeUPtr{node});
            return node;
        }
        // Helpers to populate the edges of a node using a recursion rule
        void BufferizeEdge(const Node* node)
        {
            edges_buf_.push_back(node);
        }
        Node::Edges UnbufferizeEdges()
        {
            auto edges = edges_buf_;
            edges_buf_.clear();
            return edges;
        }
        void set_root(const Node* node)
        {
            root_ = node;
        }

        std::vector<NodeUPtr> nodes_;
        std::deque<VarDef>    context_stack_;
        Node::Edges           edges_buf_;
        const Node*           root_ = nullptr;
};

}
