#pragma once

#include "lms/datamanager.h"
#include "signalhandler.h"
#include "argumenthandler.h"
#include "executionmanager.h"
#include "lms/logger.h"
#include "clock.h"
#include "pugixml.hpp"
#include "file_monitor.h"
#include "debug_server.h"

namespace lms {
namespace internal {

/**
 * @brief Main entry point into the lightweight module system.
 *
 * Create an object of this class and you will start the framwork.
 */
class Framework {
public:
    /**
     * @brief Instantiate the framework, initiate datamanager and
     * execution manager and start the main loop.
     *
     * @param arguments parsed command line arguments
     */
    Framework(const std::string &mainConfigFilePath);

    /**
     * @brief Destroy execution manager.
     */
    ~Framework();

    Profiler &profiler();

    std::shared_ptr<Service> getService(std::string const &name);

    bool isDebug() const;

    void addSearchPath(const std::string &path);

    std::string loadLogObject(std::string const &name, bool isDir);
    std::string saveLogObject(std::string const &name, bool isDir);
    bool isEnableLoad() const;
    bool isEnableSave() const;
    std::string loadPath() const;

    /**
     * @brief Synchronously execute a single cycle of this runtime.
     * @return true if cycle got executed, false if runtime is paused
     */
    bool cycle();

    void start();

    DataManager& dataManager();
    ExecutionManager &executionManager();

private:
    void updateSystem(const RuntimeInfo &info);

    ExecutionManager m_executionManager;
    DataManager m_dataManager;
    Clock m_clock;

    logging::Logger logger;

    Profiler m_profiler;
    Loader m_loader;

    extra::FileMonitor configMonitor;

    std::string mainConfigFilePath;

    bool m_running;

    std::map<std::string, std::shared_ptr<Service>> services;
    std::map<std::string, std::shared_ptr<Module>> modules;

    std::string configPath;

    static constexpr std::uint32_t MODE = 0775;
    std::string m_loadLogPath;
    std::string m_saveLogPath;
};

} // namespace internal
} // namespace lms
