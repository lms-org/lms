#ifndef LMS_EXECUTION_MANAGER_H
#define LMS_EXECUTION_MANAGER_H

#include <string>
#include <deque>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "lms/module.h"
#include "loader.h"
#include "lms/datamanager.h"
#include "executionmanager.h"
#include "lms/logger.h"
#include "lms/messaging.h"
#include "dag.h"
#include "watch_dog.h"
#include "module_channel_graph.h"

namespace lms {
class DataManager;
namespace internal {

class ExecutionManager {
public:
    ExecutionManager(Framework &runtime);
    ~ExecutionManager();

    /**cycle modules */
    void loop();

    /**
     * @brief Calling this method will cause the
     * executionmanager to run validate() in the next loop
     */
    void invalidate();

    /**
     * @brief If invalidate was called before, this method will create the
     * dependency graph.
     *
     * Sorts the modules in the cycle-list.
     */
    void validate(const std::map<std::string, std::shared_ptr<Module>> &enabledModules);

    /**
     * @brief Set the thread pool size.
     *
     * This is only used if multithreading is enabled.
     */
    void numThreads(int num);

    /**
     * @brief Set the thread pool size to an automatically determined way
     * that is believed to be the best for current system LMS is running on.
     *
     * This is only used if mulithreading is enabled.
     */
    void numThreadsAuto();

    /**
     * @brief Return the thread pool size.
     */
    int numThreads() const;

    /**
     * @brief Enable or diable multithreading.
     */
    void enabledMultithreading(bool flag);

    /**
     * @brief Return true if multithreading is enabled.
     */
    bool enabledMultithreading() const;

    WatchDog &dog();

    DataManager &getDataManager();

    void printCycleList();

    Messaging &messaging();

    /**
     * @brief Invoke configsChanged() of all enabled modules.
     */
    void fireConfigsChangedEvent();

    /**
     * @brief Return the current value of the cycle counter. The value is
     * incremented before each cycle, starting with 0.
     */
    int cycleCounter();

    void writeDAG(DotExporter &dot, const std::string &prefix);

    ModuleChannelGraph<Module *> &getModuleChannelGraph();

private:
    logging::Logger logger;

    int m_numThreads;
    bool m_multithreading;

    bool valid;

    Messaging m_messaging;
    WatchDog m_dog;

    int m_cycleCounter;

    // stuff for multithreading
    std::vector<std::thread> threadPool;
    std::mutex mutex;
    std::condition_variable cv;
    size_t numModulesToExecute;
    bool running;
    bool hasExecutableModules(int thread);
    void threadFunction(int threadNum);
    void stopRunning();

    Framework &m_runtime;

    ModuleChannelGraph<Module *> moduleChannelGraph;
    DAG<Module *> cycleList;
    DAG<Module *> cycleListTmp;
    std::vector<Module *> sortedCycleList;

    void printCycleList(DAG<Module *> &list);
};

} // namespace internal
} // namespace lms

#endif /* LMS_EXECUTION_MANAGER_H */
