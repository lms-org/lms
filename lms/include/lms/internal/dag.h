#ifndef LMS_INTERNAL_DAG_H
#define LMS_INTERNAL_DAG_H

#include <map>
#include <set>

namespace lms {
namespace internal {

/**
 * @brief Templated directed acyclic graph.
 *
 * https://en.wikipedia.org/wiki/Directed_acyclic_graph
 *
 * This implementation is not thread-safe.
 */
template<typename T>
class DAG {
private:
    typedef std::map<T, std::set<T>> GraphType;
    GraphType m_data;
public:
    /**
     * @brief Add an edge from dependency to node.
     *
     * The nodes will be created if not yet existent.
     *
     * Reflexive edges are ignored.
     *
     * @param node target node
     * @param dependency source node
     */
    void edge(T const& from, T const& to) {
        if(from != to) {
            m_data[to].insert(from);
            m_data[from];
        }
    }

    /**
     * @brief Create a node.
     *
     * Does nothing if node was already there.
     *
     * @param node node to be created
     */
    void node(T const& node) {
        m_data[node];
    }

    /**
     * @brief Remove a node and its incoming edges.
     *
     * This function is intended to be used for free nodes.
     *
     * @param node node to remove
     */
    void removeNode(T const& node) {
        m_data.erase(node);
    }

    /**
     * @brief Count the number of nodes available in this graph.
     * @return number of nodes
     */
    size_t countNodes() const {
        return m_data.size();
    }

    /**
     * @brief Check if the graph contains an edge.
     * @param node
     * @param dependency
     * @return
     */
    bool hasEdge(T const& from, T const& to) const {
        typename GraphType::const_iterator it = m_data.find(to);
        return it != m_data.end() && 1 == it->second.count(from);
    }

    /**
     * @brief Remove an edge.
     *
     * Does nothing if edge was not existent.
     *
     * @param from source node
     * @param to target node
     */
    void removeEdge(T const& from, T const& to) {
        typename GraphType::iterator it = m_data.find(to);
        if(it != m_data.end()) {
            it->second.erase(from);
        }
    }

    /**
     * @brief Check if the graph contains any free node that satifies the
     * predicate.
     *
     * The predicate must be function or lamda that takes a const& T and returns
     * bool.
     *
     * @param predicate a function of type bool (*)(T const&)
     * @return true if a free node was found, false otherwise
     */
    template<typename PredicateFn>
    bool hasFree(PredicateFn predicate) const {
        for(typename GraphType::const_iterator it = m_data.begin(); it != m_data.end(); it++) {
            if(it->second.empty() && predicate(it->first)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Search for a node having no incoming edges.
     * @param result The node value will be assigned to this parameter
     * @return true if a free node was found and result was assigned, false
     * otherwise
     */
    bool getFree(T& result) const {
        for(typename GraphType::const_iterator it = m_data.begin(); it != m_data.end(); it++) {
            if(it->second.empty()) {
                result = it->first;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Search for a node having no incoming edges and satisfying the
     * predicate.
     * @param result the found node will be place there
     * @param predicate a function of type bool (*)(T &)
     */
    template<typename PredicateFn>
    bool getFree(T& result, PredicateFn predicate) {
        for(typename GraphType::const_iterator it = m_data.begin(); it != m_data.end(); it++) {
            if(it->second.empty() && predicate(it->first)) {
                result = it->first;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Delete all edges coming from a certain node.
     * @param node source node
     */
    void removeEdgesFrom(T const& from) {
        for(auto & pair : m_data) {
            pair.second.erase(from);
        }
    }

    /**
     * @brief Check if the graph contains no nodes or dependencies
     * @return true if graph is empty, false otherwise
     */
    bool empty() const {
        return m_data.empty();
    }

    /**
     * @brief Check if the dependency graph has any circles or may come
     * into state where there are no free nodes.
     * @return true if the graph may get deadlocked, false otherwise
     */
    bool hasCycle() const {
        DAG copy(*this);

        T node;
        while(copy.getFree(node)) {
            copy.removeNode(node);
            copy.removeEdgesFrom(node);
        }

        return !copy.empty();
    }

    /**
     * @brief Delete all nodes and edges.
     */
    void clear() {
        m_data.clear();
    }

    /**
     * @brief Perform a topological sort on the graph.
     *
     * https://en.wikipedia.org/wiki/Topological_sorting
     *
     * The result parameter must a list type that supports push_back. Therefore
     * the template parameter should be std::vector, std::list or std::deque.
     *
     * @param result The sorted node list will push_back'ed to this parameter
     * @return true if sorting was successful, false if the graph contains
     * cycles
     */
    template<typename ListType>
    bool topoSort(ListType & result) {
        DAG copy(*this);

        T node;
        while(copy.getFree(node)) {
            copy.removeNode(node);
            copy.removeEdgesFrom(node);
            result.push_back(node);
        }

        return copy.empty();
    }

    /**
     * @brief Check if there is a path from one node to another via one or more
     * edges.
     * @param from start node
     * @param to end node
     * @return true if at least one path was found, false otherwise
     */
    bool hasPath(T const& from, T const& to) const {
        if(0 == m_data.count(from) || 0 == m_data.count(to)) {
            return false;
        }

        std::set<T> done, todo;

        todo.insert(to);

        while(! todo.empty()) {
            if(1 == todo.count(from)) {
                return true;
            }

            T const& work = *todo.begin();
            auto towork = m_data.find(work);

            if(towork == m_data.end()) {
                todo.erase(work);
                continue;
            }

            for(T const& x : towork->second) {
                // if node not yet done then put it todo
                if(0 == done.count(x)) {
                    todo.insert(x);
                }
            }

            // the work node is now done
            todo.erase(work);
            done.insert(work);
        }

        return false;
    }

    /**
     * @brief Remove all transitive edges from the graph.
     *
     * Transitive edges are edges that connect two nodes that are also connected
     * via a path not using this edge.
     */
    void removeTransitiveEdges() {
        bool changed;

        do {
            changed = false;

            for(auto & pair : m_data) {
                T const& to = pair.first;
                std::set<T> copy(pair.second);
                for(auto const& from : copy) {
                    pair.second.erase(from);
                    if(hasPath(from, to)) {
                        changed = true;
                    } else {
                        pair.second.insert(from);
                    }
                }
            }
        } while(changed);
    }

    /**
     * @brief Iterator to the internal data structure.
     * @return read-only begin iterator
     */
    typename GraphType::const_iterator begin() const {
        return m_data.begin();
    }

    /**
     * @brief Iterator to the internal data structure.
     * @return  read-only end iterator
     */
    typename GraphType::const_iterator end() const {
        return m_data.end();
    }
};

}  // namespace internal
}  // namespace lms

#endif // LMS_INTERNAL_DAG_H
