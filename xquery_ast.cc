#include <functional>
#include <cassert>
#include <iostream>
#include <fstream>

#include "xquery_misc.h"
#include "xquery_ast.h"

#ifdef USE_BOOST_GRAPHVIZ
#include <boost/graph/graphviz.hpp>

template <typename Container>
class graphviz_label_writer
{
    public:
        graphviz_label_writer(const Container& container) : container_{container} {}

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
    return c;
}
#endif

namespace xquery
{

void Ast::PlotGraph() const
{
#ifdef USE_BOOST_GRAPHVIZ
    using GraphEdge = std::pair<size_t, size_t>;
    using Graph = boost::adjacency_list<>;

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

    Graph g{std::begin(graph_edges), std::end(graph_edges), nodes_.size()};
    std::ofstream fs{filename};

    if ( !fs.good())
        throw std::ios_base::failure{"Could not open " + filename};
    boost::write_graphviz(fs, g, ::make_graphviz_label_writer(nodes_));
    fs.close();
    std::cout << "AST generated successfully"_green << std::endl;
#else
    std::cerr << "Graphiz plotting is not supported. "_yellow <<
      "Try compiling with USE_BOOST_GRAPHVIZ=true"_yellow << std::endl;
#endif
}

void Ast::Eval() const
{
    auto output = root_->Eval();

    assert(output.type == Node::EvalResult::NODES);
    for (const auto node : output.nodes) {
        std::cout << "Name: \"" << node->get_name() << "\"";

        // Element attribute
        auto elem = dynamic_cast<const xml::Element*>(node);
        if (elem) {
            auto attr = elem->get_attribute("attr");
            if (attr) std::cout << ", Attr: \"" << attr->get_value() << "\"";
        }
        // Text
        auto text = dynamic_cast<const xml::TextNode*>(node);
        if (text) std::cout << ", Text: \"" << text->get_content() << "\"";

        std::cout << std::endl;
    }
}

}
