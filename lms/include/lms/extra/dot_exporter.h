#ifndef LMS_EXTRA_DOT_EXPORTER_H
#define LMS_EXTRA_DOT_EXPORTER_H

#include <iostream>
#include <string>
#include <stack>

namespace lms {
namespace extra {

/**
 * @brief Export a directed or undirected graph to a file in dot format.
 *
 * For more information:
 * https://en.wikipedia.org/wiki/DOT_(graph_description_language)
 *
 * General usage:
 * std::ofstream ofs("file.dot");
 * DotExporter dot(ofs);
 * dot.graph("MyGraph");
 * dot.node("A");
 * dot.node("B");
 * dot.edge("A", "B");
 * dot.endGraph();
 * ofs.close();
 */
class DotExporter {
public:
    /**
     * @brief Node shape types.
     */
    enum class Shape {
        BOX, OVAL, CIRCLE
    };

    /**
     * @brief Error types returned by lastError().
     */
    enum class Error {
        OK, STACK_EMPTY, UNEXPECTED_STACK_TOP
    };

    /**
     * @brief Create a dot exporter on an std::ostream. The stream is not
     * managed by DotExporter and must be closed externally.
     * @param os output stream, can be std::ofstream
     */
    DotExporter(std::ostream &os);

    /**
     * @brief Start a directed graph.
     * @param name graph name
     */
    void startDigraph(const std::string &name);

    /**
     * @brief End a directed graph.
     */
    void endDigraph();

    /**
     * @brief Start an undirected graph.
     * @param name graph name
     */
    void startGraph(const std::string &name);

    /**
     * @brief End an undirected graph.
     */
    void endGraph();

    /**
     * @brief Start a subgraph.
     * @param name subgraph name
     */
    void startSubgraph(const std::string &name);

    /**
     * @brief End a subgraph
     */
    void endSubgraph();

    /**
     * @brief Make an edge between the two nodes. In undirected graphs, the
     * from and to parameter can be changed without making a difference.
     *
     * @param from source node
     * @param to target node
     */
    void edge(const std::string &from, const std::string &to);

    /**
     * @brief Create the node with the given name. This is usually not
     * necessary in the dot format, since it automatically creates nodes
     * that are referenced in edge definitions. You should call this method
     * if your node has custom attributes like a label or shape.
     *
     * @param name node name
     */
    void node(const std::string &name);

    /**
     * @brief Set the label to be used for nodes and edges.
     * @param value label text
     */
    void label(const std::string &value);

    /**
     * @brief Set the shape to be used for nodes.
     * @param value shape type
     */
    void shape(Shape value);

    /**
     * @brief Reset the options that were by label() and shape().
     */
    void reset();

    /**
     * @brief Get the error of the last method.
     * @return error type
     */
    Error lastError() const;

    /**
     * @brief Reset the value returned by lastError() to OK.
     */
    void resetError();
private:
    void indent();

    enum class StackType {
        GRAPH, DIGRAPH, SUBGRAPH
    };

    std::ostream &m_os;
    std::stack<StackType> m_stack;
    Error m_error;

    std::string m_label;
    Shape m_shape;
};

std::ostream& operator << (std::ostream& os, DotExporter::Shape type);

std::ostream& operator << (std::ostream& os, DotExporter::Error error);

}  // namespace extra
}  // namespace lms

#endif // PROJECT_DOT_EXPORTER_H
