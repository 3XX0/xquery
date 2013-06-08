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
    friend class ContextIterator;

    public:
        using Edges = std::vector<Node*>;
        using const_iterator = Edges::const_iterator;

        struct EvalResult;

        virtual ~Node() = default;

        // Throws `std::runtime_error' or `xml::validity_error'
        virtual EvalResult Eval(const EvalResult& res) const = 0;
        virtual void Rewrite()
        {
            for (auto edge : edges_)
                if (edge)
                    edge->Rewrite();
        }

        const_iterator begin() const
        {
            return std::begin(edges_);
        }
        const_iterator end() const
        {
            return std::end(edges_);
        }

        void AddEdge(Node* edge)
        {
            edges_.push_back(edge);
        }
        void DeleteEdge(const Node* edge)
        {
            auto it = std::find(std::begin(edges_), std::end(edges_), edge);
            if (it != std::end(edges_))
                edges_.erase(it);
        }

        const std::string& label() const
        {
            return label_;
        }
        Edges& edges()
        {
            return edges_;
        }
        Node* parent()
        {
            return parent_;
        }

    protected:
        Node() = default;
        Node(Edges&& edges) : edges_{std::move(edges)}, ast_{nullptr}
        {
            for (auto edge : edges_)
                if (edge)
                    edge->set_parent(this);
        }

        void set_label(const std::string& label)
        {
            label_ = label;
        }

        Node*       parent_;
        Edges       edges_;
        std::string label_;
        size_t      id_ = 0;
        Ast*        ast_;

    private:
        void set_parent(Node* parent)
        {
            parent_ = parent;
        }

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

    using NodeUPtr = std::unique_ptr<Node>;
    #define SCOPE_DELIM "{SD}"

    public:
        using VarDef = std::pair<std::string, xml::NodeList>;
        using Context = std::vector<VarDef>;
        using ContextStack = std::deque<VarDef>;

        Ast()
        {
            collector_.create_root_node("collector");
        }
        ~Ast() = default;

        void PlotGraph() const; // Throws `std::ios_base'
        void Evaluate() const;  // Throws `std::runtime_error'
        void Rewrite();

        /*
         * Node specific
         */
        Node* AddNode(Node* node)
        {
            node->set_id(nodes_.size());
            node->set_ast(this);
            nodes_.push_back(NodeUPtr{node});
            return node;
        }
        void DeleteNode(Node* node)
        {
            for (auto edge : node->edges())
                DeleteNode(edge);
            auto it = std::find_if(std::begin(nodes_), std::end(nodes_),
              [node](const NodeUPtr& ptr) {return ptr.get() == node;});
            if (it != std::end(nodes_))
                nodes_.erase(it);
            for (size_t i = 0; i < nodes_.size(); ++i) // ID renumbering
                nodes_[i]->set_id(i);
        }
        xml::Element* CollectElement(const std::string& name)
        {
            return collector_.get_root_node()->add_child(name);
        }
        xml::TextNode* CollectTextNode(const std::string& content)
        {
            static size_t id = 0;

            auto node = collector_.get_root_node()->add_child("#" + std::to_string(id++));
            return node->add_child_text(content);
        }
        void CtxNew()
        {
            context_stack_.emplace_front(SCOPE_DELIM, xml::NodeList{});
        }
        void CtxDestroy()
        {
            auto it = std::find_if(std::begin(context_stack_), std::end(context_stack_),
              [this](const VarDef& def) { return def.first == SCOPE_DELIM; });
            context_stack_.erase(std::begin(context_stack_), ++it);
        }
        void CtxPushVarDef(const std::string& varname, xml::NodeList&& nodes)
        {
            context_stack_.emplace_front(varname, std::move(nodes));
        }
        VarDef CtxPopVarDef()
        {
            auto vdef = context_stack_.front();
            context_stack_.pop_front();
            return vdef;
        }
        const xml::NodeList& CtxFindVarDef(const std::string& varname) // Throws
        {
            auto it = std::find_if(std::begin(context_stack_), std::end(context_stack_),
              [this, &varname](const VarDef& def) { return def.first == varname; });
            if (it == std::end(context_stack_))
                throw std::runtime_error("Undefined variable " + varname);
            return it->second;
        }

    private:
        /*
         * Parser specific
         */

        // Helpers to populate the edges of a node using a recursion rule
        void BufferizeEdge(Node* node)
        {
            edges_buf_.push_back(node);
        }
        Node::Edges UnbufferizeEdges()
        {
            auto edges = edges_buf_;
            edges_buf_.clear();
            return edges;
        }
        void set_root(Node* node)
        {
            root_ = node;
        }

        std::vector<NodeUPtr> nodes_;
        ContextStack          context_stack_;
        Node::Edges           edges_buf_;
        Node*                 root_ = nullptr;
        xml::Document         collector_; // XXX: xmlpp pseudo factory
        mutable xml::Document output_doc_;
};

}
