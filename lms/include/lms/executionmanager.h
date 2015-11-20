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
#include "lms/profiler.h"
#include "lms/messaging.h"

namespace lms {

class DataManager;
class Loader;

class ExecutionManager {
public:

    ExecutionManager(Profiler &profiler, Runtime &runtime);
    ~ExecutionManager();

    /**cycle modules */
    void loop();

    /**
     * @brief Add a new module to the list of available modules.
     */
    void installModule(std::shared_ptr<ModuleWrapper> mod);

    /**
     * @brief Append the given module to a list of new modules. After the
     * next cycle the module will be added to the available list or an existing
     * module may be updated. Use updateOrInstall() for this.
     *
     * This method is thread-safe.
     *
     * @param mod module to install or update
     */
    void bufferModule(std::shared_ptr<ModuleWrapper> mod);

    /**
     * @brief Update or install previously buffered modules.
     */
    void updateOrInstall();

    /**
     * @brief Disable all modules that are currently enabled.
     */
    void disableAllModules();

    /**
     * @brief Enable module with the given name, add it to the cycle-queue.
     *
     * @param name name of the module that should be enabled
     * @param minLogLevel minimum logging level
     */
    void enableModule(const std::string &name, logging::Level minLogLevel
                      = logging::Level::ALL);

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

    DataManager& getDataManager();

    void printCycleList();

    Profiler& profiler();

    Messaging& messaging();

    const ModuleList& getEnabledModules() const;

    /**
     * @brief Invoke configsChanged() of all enabled modules.
     */
    void fireConfigsChangedEvent();

    /**
     * @brief Return the current value of the cycle counter. The value is
     * incremented before each cycle, starting with 0.
     */
    int cycleCounter();

    typedef std::pair<std::string, logging::Level> ModuleToEnable;
    typedef std::vector<ModuleToEnable> EnableConfig;

    EnableConfig& config(std::string const& name);

    void useConfig(std::string const& name);

    void writeDAG(lms::extra::DotExporter &dot, const std::string &prefix);
private:
    std::string m_runtimeName;
    logging::Logger logger;

    int m_numThreads;
    bool m_multithreading;

    bool valid;

    DataManager dataManager;
    Messaging m_messaging;

    int m_cycleCounter;

    // stuff for multithreading
    std::vector<std::thread> threadPool;
    std::mutex mutex;
    std::condition_variable cv;
    int numModulesToExecute;
    bool running;
    bool hasExecutableModules(int thread);
    void threadFunction(int threadNum);
    void stopRunning();

    Profiler& m_profiler;
    Runtime & m_runtime;

    /**
     * @brief enabledModules contains all loaded Modules
     */
    ModuleList enabledModules;

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
    ModuleList available;

    std::map<std::string, EnableConfig> m_configs;

    std::mutex updateMutex;
    ModuleList update;

    /**
     * @brief Call this method for sorting the cycleList.
     *
     * As this method isn't called often I won't care about performance but
     * readability. If you have to call this method often you might have to
     * improve it.
     */
    void sort();
    void sortModules();
    void addModuleDependency(std::shared_ptr<ModuleWrapper> dependent, std::shared_ptr<ModuleWrapper> independent);
};

}  // namespace lms

#endif /* LMS_EXECUTION_MANAGER_H */
