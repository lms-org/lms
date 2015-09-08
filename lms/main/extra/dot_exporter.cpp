#include "lms/extra/dot_exporter.h"

namespace lms {
namespace extra {

DotExporter::DotExporter(std::ostream &os) : m_os(os) {
    reset();
}

void DotExporter::startDigraph(const std::string &name) {
    m_stack.push(StackType::DIGRAPH);
    m_os << "digraph " << name << " {\n";
}

void DotExporter::endDigraph() {
    if(m_stack.empty() || m_stack.top() != StackType::DIGRAPH) {
        // TODO error handling
    }

    m_stack.pop();
    m_os << "}\n";
}

void DotExporter::startGraph(const std::string &name) {
    m_stack.push(StackType::GRAPH);
    m_os << "graph " << name << " {\n";
}

void DotExporter::endGraph() {
    if(m_stack.empty() || m_stack.top() != StackType::GRAPH) {
        // TODO error handling
    }

    m_stack.pop();
    m_os << "}\n";
}

void DotExporter::edge(const std::string &from, const std::string &to) {
    if(m_stack.empty()) {
        // TODO error handling
    }

    indent();
    m_os << from;
    if(m_stack.top() == StackType::GRAPH) {
        m_os << " -- ";
    } else {
        m_os << " -> ";
    }
    m_os << to << " [label=\"" << m_label << "\"];\n";
}

void DotExporter::node(const std::string &name) {
    indent();
    m_os << name << " [label=\"" << m_label << "\", shape=\"" << m_shape << "\"];\n";
}

void DotExporter::indent() {
    for(size_t i = 0; i < m_stack.size(); i++) {
        m_os << "  ";
    }
}

void DotExporter::label(const std::string &value) {
    m_label = value;
}

void DotExporter::shape(ShapeType value) {
    m_shape = value;
}

void DotExporter::reset() {
    m_label.clear();
    m_shape = ShapeType::OVAL;
}

std::ostream& operator << (std::ostream& os, DotExporter::ShapeType type) {
    typedef DotExporter::ShapeType T;

    switch(type) {
        case T::BOX: os << "box"; break;
        case T::OVAL: os << "oval"; break;
        case T::CIRCLE: os << "circle"; break;
    }

    return os;
}

}  // namespace extra
}  // namespace lms
