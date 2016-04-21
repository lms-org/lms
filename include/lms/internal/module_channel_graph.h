#ifndef LMS_INTERNAL_MODULE_CHANNEL_GRAPH_H
#define LMS_INTERNAL_MODULE_CHANNEL_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "dag.h"
#include "dot_exporter.h"

namespace lms {
namespace internal {

template<typename T>
class ModuleChannelGraph {
public:
    enum class Permission {
        READ, WRITE
    };

    struct Access {
        T module;
        Permission permission;
        int priority;
    };

    typedef std::unordered_map<std::string, std::vector<Access>> GraphType;
    GraphType m_data;

    void readChannel(const std::string &channel, const T &module, int priority = 0) {
        if(!isReaderOrWriter(channel, module)) {
            m_data[channel].push_back({module, Permission::READ, priority});
        }
    }

    void writeChannel(const std::string &channel, const T &module, int priority = 0) {
        if(!isReaderOrWriter(channel, module)) {
            m_data[channel].push_back({module, Permission::WRITE, priority});
        }
    }

    bool isReaderOrWriter(const std::string &channel, const T &module) const {
        auto it = m_data.find(channel);
        if(it != m_data.end()) {
            for(const auto &pair : it->second) {
                if(pair.module == module) {
                    return true;
                }
            }
        }
        return false;
    }

    void clear() {
        m_data.clear();
    }

    DAG<T> generateDAG() const {
        DAG<T> dag;

        // Iterate over all channels
        for(const auto &channel : m_data) {
            // Iterate over all module combination accessing this channel
            for(auto it = channel.second.begin(); it != channel.second.end(); ++it) {
                const T &mw1 = it->module;
                int prio1 = it->priority;

                for(auto jt = it + 1; jt != channel.second.end(); ++jt) {
                    const T &mw2 = jt->module;
                    int prio2 = jt->priority;

                    if(prio2 > prio1) {
                        dag.edge(mw2, mw1);
                    } else if(prio1 > prio2) {
                        dag.edge(mw1, mw2);
                    } else {
                        //check if it's reader vs writer
                        bool mw1Write = it->permission == Permission::WRITE;
                        bool mw2Write = jt->permission == Permission::WRITE;

                        if(mw1Write && !mw2Write){
                            dag.edge(mw1, mw2);
                        }else if(!mw1Write && mw2Write){
                            dag.edge(mw2, mw1);
                        }

                        // TODO both writers -> throw exception
                    }
                }
            }
        }

        return dag;
    }

    typename GraphType::const_iterator begin() const {
        return m_data.begin();
    }

    typename GraphType::const_iterator end() const {
        return m_data.end();
    }
};

}  // namespace internal
}  // namespace lms

#endif
