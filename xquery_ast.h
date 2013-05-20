#pragma once

#include <memory>
#include <vector>

#include "xquery_xml.h"

namespace xquery
{

class Node
{
    public:
        union EvalResult
        {
            EvalResult() = default;
            EvalResult(xml::Elements&& elems) : elements(elems) {}
            EvalResult(bool cond) : condition(cond) {}
            ~EvalResult() = default;

            xml::Elements elements;
            bool          condition;
        };

        typedef std::vector<const Node*>::const_iterator const_iterator;

    protected:
        typedef std::vector<const Node*> Edges;

    public:
        Node() = default;
        Node(Edges&& edges) : edges_(edges) {}
        virtual ~Node()
        {
            edges_.clear();
        }

        const_iterator begin() const
        {
            return std::begin(edges_);
        }
        const_iterator end() const
        {
            return std::end(edges_);
        }

        virtual EvalResult Eval(const EvalResult& res) const = 0;
        void AddEdge(const Node* node) const
        {
            edges_.push_back(node);
        }

        void set_label(const std::string& label)
        {
            label_ = label;
        }
        const std::string& label() const
        {
            return label_;
        }
        void set_id(size_t id)
        {
            id_ = id;
        }
        size_t id() const
        {
            return id_;
        }

    protected:
        mutable Edges edges_;
        std::string   label_;
        size_t        id_ = 0;
};

class Ast
{
    typedef std::unique_ptr<const Node> NodeUPtr;
    typedef std::vector<const Node*>    Edges;

    public:
        Ast() = default;
        ~Ast()
        {
            nodes_.clear();
        }

        const Node* AddNode(Node* node)
        {
            node->set_id(nodes_.size());
            nodes_.push_back(NodeUPtr(node));
            return node;
        }
        // Helpers to populate the edges of a node using a recursion rule
        void BufferizeEdge(const Node* node)
        {
            edges_buf_.push_back(node);
        }
        Edges UnbufferizeEdges()
        {
            auto edges = edges_buf_;
            edges_buf_.clear();
            return edges;
        }
        void PlotGraph() const;

        void set_root(const Node* node)
        {
            root_ = node;
        }
        const Node* root() const
        {
            return root_;
        }

    private:
        std::vector<NodeUPtr> nodes_;
        Edges                 edges_buf_;
        const Node*           root_ = nullptr;
};

}
