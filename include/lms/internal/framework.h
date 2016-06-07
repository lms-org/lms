#ifndef LMS_FRAMEWORK_H
#define LMS_FRAMEWORK_H

#include "../datamanager.h"
#include "signalhandler.h"
#include "argumenthandler.h"
#include "executionmanager.h"
#include "../logger.h"
#include "clock.h"
#include "pugixml.hpp"
#include "file_monitor.h"
#include "runtime.h"
#include "debug_server.h"

namespace lms {
namespace internal {

/**
 * @brief Main entry point into the lightweight module system.
 *
 * Create an object of this class and you will start the framwork.
 */
class Framework : public SignalHandler::Listener {
public:
    /**
     * @brief Instantiate the framework, initiate datamanager and
     * execution manager and start the main loop.
     *
     * @param arguments parsed command line arguments
     */
    Framework(const ArgumentHandler &arguments);

    /**
     * @brief Destroy execution manager.
     */
    ~Framework();

    void registerRuntime(Runtime *runtime);

    Runtime *getRuntimeByName(std::string const &name);

    bool hasRuntime(std::string const &name);

    ArgumentHandler const &getArgumentHandler();

    void exportGraphs();

    Profiler &profiler();

    Loader &moduleLoader();

    std::shared_ptr<ServiceWrapper> getServiceWrapper(std::string const &name);

    void installService(std::shared_ptr<ServiceWrapper> service);
    void reloadService(std::shared_ptr<ServiceWrapper> service);

    bool isDebug() const;

    std::string loadLogObject(std::string const &name, bool isDir);
    std::string saveLogObject(std::string const &name, bool isDir);
    bool isEnableLoad() const;
    bool isEnableSave() const;
    std::string loadPath() const;

private:
    bool exportGraphsHelper(bool isExecOrData);

    logging::Logger logger;

    ArgumentHandler argumentHandler;
    Profiler m_profiler;
    Loader m_moduleLoader;
    Loader m_serviceLoader;

    extra::FileMonitor configMonitor;

    bool m_running;

    std::map<std::string, std::unique_ptr<Runtime>> runtimes;
    std::map<std::string, std::shared_ptr<ServiceWrapper>> services;

    std::string configPath;

    /**
     * @brief signal called by the system (Segfaults etc)
     * @param s
     */
    void signal(int s);

    static constexpr std::uint32_t MODE = 0775;
    std::string m_loadLogPath;
    std::string m_saveLogPath;

    DebugServer m_debugServer;
};

} // namespace internal
} // namespace lms

#endif /* LMS_FRAMEWORK_H */
