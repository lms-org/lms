#ifndef LMS_FRAMEWORK_H
#define LMS_FRAMEWORK_H

#include <lms/datamanager.h>
#include <lms/signalhandler.h>
#include <lms/argumenthandler.h>
#include <lms/executionmanager.h>
#include <lms/logger.h>
#include "lms/clock.h"
#include "pugixml.hpp"
#include "lms/extra/file_monitor.h"
#include "lms/deprecated.h"
#include "lms/runtime.h"

namespace lms {

/**
 * @brief Main entry point into the lightweight module system.
 *
 * Create an object of this class and you will start the framwork.
 */
class Framework : public SignalHandler::Listener {
public:
    /**
     * @brief executionPath absolute path to running framework (folder that contains core and external)
     */
    DEPRECATED static std::string programDirectory();

    /**
     * @brief Framework::externalDirectory absolute path to the external directory
     *
     * This attribute is deprecated.
     */

    static std::string externalDirectory;
    /**
     * @brief Framework::configsDirectory absolute path to the configs directory
     *
     * This attribute is deprecated.
     */
    static std::string configsDirectory;

    /**
     * @brief Instantiate the framework, initiate datamanager and
     * execution manager and start the main loop.
     *
     * @param arguments parsed command line arguments
     */
    Framework(const ArgumentHandler& arguments);

    /**
     * @brief Destroy execution manager.
     */
    ~Framework();

    void registerRuntime(Runtime *runtime);

    Runtime* getRuntimeByName(std::string const& name);

    bool hasRuntime(std::string const& name);

    ArgumentHandler const& getArgumentHandler();

    void exportGraphs();

    Profiler& profiler();

    Loader<Module>& moduleLoader();

    std::shared_ptr<ServiceWrapper> getServiceWrapper(std::string const& name);

    void installService(std::shared_ptr<ServiceWrapper> service);
    void reloadService(std::shared_ptr<ServiceWrapper> service);
private:
    bool exportGraphsHelper(std::string const& path, bool isExecOrData);

    logging::Logger logger;

    ArgumentHandler argumentHandler;
    Profiler m_profiler;
    Loader<Module> m_moduleLoader;
    Loader<Service> m_serviceLoader;

    extra::FileMonitor configMonitor;

    bool m_running;

    std::map<std::string, std::unique_ptr<Runtime>> runtimes;
    std::map<std::string, std::shared_ptr<ServiceWrapper>> services;

    /**
     * @brief signal called by the system (Segfaults etc)
     * @param s
     */
    void signal(int s);
};

}  // namespace lms

#endif /* LMS_FRAMEWORK_H */
