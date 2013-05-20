#pragma once

#include <cassert>
#include <memory>
#include <vector>
#include <functional>
#include <fstream>

#ifdef USE_BOOST_GRAPHVIZ
#include <boost/graph/graphviz.hpp>

template <typename Container>
class graphviz_label_writer
{
    public:
        graphviz_label_writer(const Container& container) : container_(container) {}

        template <class VertexId>
        void operator()(std::ostream& out, const VertexId& id) const
        {
            out << "[label=\"" << container_[id]->label() << "\"]";
        }

    private:
        const Container& container_;
};

template <typename Container>
inline graphviz_label_writer<Container> make_graphviz_label_writer(const Container& c)
{
    return {c};
}
#endif

namespace xquery
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
        mutable std::vector<const Node*> edges_;
        std::string                      label_;
        size_t                           id_ = 0;
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
        void PlotGraph() const
        {
#ifdef USE_BOOST_GRAPHVIZ
            typedef std::pair<size_t, size_t> GraphEdge;
            typedef boost::adjacency_list<> Graph;

            std::vector<GraphEdge> graph_edges;
            std::string filename = "ast.dot";

            std::function<void (const Node*)> trace =
              [&](const Node* node) {
                  const auto kParentId = node->id();
                  for (auto child : *node) {
                      graph_edges.push_back({kParentId, child->id()});
                      trace(child);
                  }
              };

            assert(root_ != nullptr);
            trace(root_);
            Graph g(std::begin(graph_edges), std::end(graph_edges), nodes_.size());
            std::ofstream fs(filename);
            if ( !fs.good())
                throw std::ios_base::failure("Could not open " + filename);
            boost::write_graphviz(fs, g, ::make_graphviz_label_writer(nodes_));
            fs.close();
#else
            std::cerr << "Graphiz plotting is not supported. " <<
              "Try compiling with USE_BOOST_GRAPHVIZ=true" << std::endl;
#endif
        }

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
