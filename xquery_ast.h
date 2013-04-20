#pragma once

#include <cassert>
#include <memory>
#include <vector>
#include <functional>

namespace XQuery
{

class Node
{
    public:
        typedef std::vector<const Node*>::const_iterator const_iterator;

        Node() = default;
        Node(std::vector<const Node*>&& edges) : edges_(edges) {}
        virtual ~Node()
        {
            edges_.clear();
        }

        virtual void trace() const = 0;
        const_iterator begin() const
        {
            return std::begin(edges_);
        }
        const_iterator end() const
        {
            return std::end(edges_);
        }

    protected:
        std::vector<const Node*> edges_;
};

class Ast
{
    typedef std::unique_ptr<const Node> NodeUPtr;

    public:
        Ast() = default;
        ~Ast()
        {
            nodes_.clear();
        }
        const Node* addNode(const Node* node)
        {
            nodes_.push_back(NodeUPtr(node));
            return node;
        }
        void setRoot(const Node* node)
        {
            root_ = node;
        }
        void print() const
        {
            assert(root_ != nullptr);

            std::function<void (const Node*)> trace =
              [&trace](const Node* node) {
                  node->trace();
                  for (auto children : *node) {
                      trace(children);
                  }
              };
            trace(root_);
        }

    private:
        const Node*           root_ = nullptr;
        std::vector<NodeUPtr> nodes_;
};

}
