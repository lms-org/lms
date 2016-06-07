#include "lms/internal/dot_exporter.h"

namespace lms {
namespace internal {

DotExporter::DotExporter(std::ostream &os) : m_os(os), m_error(Error::OK) {
    reset();
}

void DotExporter::startDigraph(const std::string &name) {
    m_stack.push(StackType::DIGRAPH);
    m_os << "digraph " << name << " {\n";
}

void DotExporter::endDigraph() {
    if (m_stack.empty() || m_stack.top() != StackType::DIGRAPH) {
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
    if (m_stack.empty()) {
        m_error = Error::STACK_EMPTY;
        return;
    }

    if (m_stack.top() != StackType::GRAPH) {
        m_error = Error::UNEXPECTED_STACK_TOP;
        return;
    }

    m_stack.pop();
    m_os << "}\n";
}

void DotExporter::startSubgraph(const std::string &name) {
    indent();
    m_os << "subgraph cluster_" << name << " {\n";

    m_stack.push(StackType::SUBGRAPH);

    indent();
    m_os << "label = \"" << name << "\";\n";
}

void DotExporter::endSubgraph() {
    if (m_stack.empty()) {
        m_error = Error::STACK_EMPTY;
        return;
    }

    if (m_stack.top() != StackType::SUBGRAPH) {
        m_error = Error::UNEXPECTED_STACK_TOP;
        return;
    }

    m_stack.pop();
    m_os << "}\n";
}

void DotExporter::edge(const std::string &from, const std::string &to) {
    if (m_stack.empty()) {
        m_error = Error::STACK_EMPTY;
        return;
    }

    indent();
    m_os << from;
    if (m_stack.top() == StackType::GRAPH) {
        m_os << " -- ";
    } else {
        m_os << " -> ";
    }
    m_os << to;

    if (!m_label.empty()) {
        m_os << " [label=\"" << m_label << "\"]";
    }
    m_os << ";\n";
}

void DotExporter::node(const std::string &name) {
    indent();
    m_os << name << " [label=\"" << m_label << "\", shape=\"" << m_shape
         << "\"];\n";
}

void DotExporter::indent() {
    for (size_t i = 0; i < m_stack.size(); i++) {
        m_os << "  ";
    }
}

void DotExporter::label(const std::string &value) { m_label = value; }

void DotExporter::shape(Shape value) { m_shape = value; }

void DotExporter::reset() {
    m_label.clear();
    m_shape = Shape::OVAL;
}

DotExporter::Error DotExporter::lastError() const { return m_error; }

void DotExporter::resetError() { m_error = Error::OK; }

std::ostream &operator<<(std::ostream &os, DotExporter::Shape type) {
    typedef DotExporter::Shape T;

    switch (type) {
    case T::BOX:
        os << "box";
        break;
    case T::OVAL:
        os << "oval";
        break;
    case T::CIRCLE:
        os << "circle";
        break;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, DotExporter::Error error) {
    typedef DotExporter::Error T;

    switch (error) {
    case T::OK:
        os << "OK";
        break;
    case T::STACK_EMPTY:
        os << "Stack empty";
        break;
    case T::UNEXPECTED_STACK_TOP:
        os << "Unexpected stack top";
        break;
    }

    return os;
}

} // namespace internal
} // namespace lms
