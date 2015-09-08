#ifndef LMS_EXTRA_DOT_EXPORTER_H
#define LMS_EXTRA_DOT_EXPORTER_H

#include <iostream>
#include <string>
#include <stack>

namespace lms {
namespace extra {

class DotExporter {
public:
    enum class ShapeType {
        BOX, OVAL, CIRCLE
    };

    enum class ErrorType {
        OK
    };

    DotExporter(std::ostream &os);

    void startDigraph(const std::string &name);
    void endDigraph();

    void startGraph(const std::string &name);
    void endGraph();

    void edge(const std::string &from, const std::string &to);

    void node(const std::string &name);

    void label(const std::string &value);
    void shape(ShapeType value);
    void reset();
private:
    void indent();

    enum class StackType {
        GRAPH, DIGRAPH
    };

    std::ostream &m_os;
    std::stack<StackType> m_stack;

    std::string m_label;
    ShapeType m_shape;
};

std::ostream& operator << (std::ostream& os, DotExporter::ShapeType type);

}  // namespace extra
}  // namespace lms

#endif // PROJECT_DOT_EXPORTER_H
