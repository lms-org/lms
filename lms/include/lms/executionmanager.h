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
#include "lms/loader.h"
#include "lms/datamanager.h"
#include "lms/executionmanager.h"
#include "lms/logger.h"
#include "lms/type/framework_info.h"

namespace lms {

class DataManager;
class Loader;

class ExecutionManager {
public:

    ExecutionManager(logging::Logger &rootLogger);
    ~ExecutionManager();

    /**cycle modules */
    void loop();

    /**
     * @brief Add a new module to the list of available modules.
     */
    void addAvailableModule(const Loader::module_entry &mod);

    /**
     * @brief Enable module with the given name, add it to the cycle-queue.
     *
     * @param name name of the module that should be enabled
     * @param minLogLevel minimum logging level
     */
    void enableModule(const std::string &name, logging::LogLevel minLogLevel
                      = logging::SMALLEST_LEVEL);

    /**
     * @brief Disable module with the given name, remove it from the
     * cycle-queue.
     *
     * @param name name of the module that should be disabled
     * @return true if disabling was successful, false otherwise
     */
    bool disableModule(const std::string &name);

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
    void validate();

    /**
     * @brief Set the thread pool size.
     */
    void setMaxThreads(int maxThreads);

    DataManager& getDataManager();

    void printCycleList();

    void enableProfiling(bool enable);
    bool enableProfiling() const;
private:
    logging::Logger &rootLogger;
    logging::ChildLogger logger;

    int maxThreads;
    bool valid;

    Loader loader;
    DataManager dataManager;
    Messaging messaging;

    // stuff for multithreading
    std::vector<std::thread> threadPool;
    std::mutex mutex;
    std::condition_variable cv;
    int numModulesToExecute;
    bool running;
    bool hasExecutableModules(int thread);
    void threadFunction(int threadNum);
    void stopRunning();

    bool m_enabledProfiling;
    type::FrameworkInfo frameworkInfo;

    /**
     * @brief enabledModules contains all loaded Modules
     */
    std::vector<Module*> enabledModules;

    /**
     * @brief cycleListT the cycleListType
     */
    typedef std::vector<std::vector<Module*>> cycleListType;
    cycleListType cycleList;
    cycleListType cycleListTmp;

    void printCycleList(cycleListType &list);

    /**
     * @brief available contains all Modules which can be loaded
     */
    Loader::moduleList available;

    /**
     * @brief Call this method for sorting the cycleList.
     *
     * As this method isn't called often I won't care about performance but
     * readability. If you have to call this method often you might have to
     * improve it.
     */
    void sort();

    /**
     * @brief sortByDataChannel Sorts enableModules into cycleList. Call this
     * method before sortByPriority().
     *
     * As this method isn't called often I won't care about performance but
     * readability. If you have to call this method often you might have to
     * improve it.
     */
    void sortByDataChannel();

    /**
     * @brief Sorts loadedModules into cycleList
     *
     * As this method isn't called often I won't care about performance but
     * readability. If you have to call this method often you might have to
     * improve it.
     */
    void sortByPriority();

};

}  // namespace lms

#endif /* LMS_EXECUTION_MANAGER_H */
