#pragma once

#include <memory>
#include <vector>

#include "xquery_xml.h"
#include "xquery_misc.h"

namespace xquery
{

class Node : public NonCopyable, public NonMoveable
{
    friend class Ast;

    public:
        using Edges = std::vector<const Node*>;
        using const_iterator = Edges::const_iterator;

        struct EvalResult : public NonCopyable
        {
            enum UnionType
            {
                NODES,
                COND,
                NONE
            };

            EvalResult() : type{NONE} {};
            EvalResult(xml::NodeList nl) : nodes{std::move(nl)}, type{NODES} {}
            EvalResult(bool cond) : condition{cond}, type{COND} {}
            EvalResult(EvalResult&& res) : type{res.type}
            {
                if (type == NODES)
                    new (&nodes) xml::NodeList{std::move(res.nodes)};
                else if (type == COND)
                    condition = res.condition;
            }
            ~EvalResult()
            {
                using NodeList = xml::NodeList;

                if (type == NODES)
                    nodes.~NodeList();
            }

            union {
                xml::NodeList nodes;
                bool          condition;
            };
            UnionType         type;
        };

        virtual ~Node() = default;

        // Throws `std::runtime_error'
        virtual EvalResult Eval(EvalResult&& res = {}) const = 0;

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
        Node(Edges&& edges) : edges_{std::move(edges)} {}

        void set_label(const std::string& label)
        {
            label_ = label;
        }

        mutable Edges edges_;
        std::string   label_;
        size_t        id_ = 0;

    private:
        /*
         * Ast specific
         */
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

    public:
        Ast() = default;
        ~Ast() = default;

        void PlotGraph() const; // Throws `std::ios_base'
        void Eval() const;      // Throws `std::runtime_error'

    private:
        /*
         * Parser specific
         */
        const Node* AddNode(Node* node)
        {
            node->set_id(nodes_.size());
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
        Node::Edges           edges_buf_;
        const Node*           root_ = nullptr;
};

}
