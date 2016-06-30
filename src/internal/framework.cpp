#include <lms/internal/framework.h>
#include <lms/internal/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <csignal>
#include <map>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include "lms/internal/backtrace_formatter.h"
#include "lms/logger.h"
#include "lms/time.h"
#include "lms/config.h"
#include "lms/internal/string.h"
#include "lms/internal/xml_parser.h"
#include "lms/internal/colors.h"
#include "lms/definitions.h"
#include "lms/internal/runtime.h"
#include "lms/internal/os.h"
#include "lms/logging/debug_server_sink.h"
#include "lms/internal/dot_exporter.h"
#include "lms/internal/viz.h"

namespace lms {
namespace internal {

Framework::Framework(const ArgumentHandler &arguments)
    : logger("lms.Framework"), argumentHandler(arguments), m_running(false) {

    logging::Context &ctx = logging::Context::getDefault();

    if (!arguments.argQuiet) {
        ctx.appendSink(new logging::ConsoleSink(std::cout));
    }

    if (!arguments.argLogFile.empty()) {
        ctx.appendSink(new logging::FileSink(arguments.argLogFile));
    }

    SignalHandler::getInstance()
        .addListener(SIGINT, this)
        .addListener(SIGSEGV, this)
        .addListener(SIGUSR1, this);

    if (arguments.argEnableDebugServer) {
        if (arguments.argDebugServerBind.find('/') == std::string::npos) {
            m_debugServer.useDualstack(
                atoi(arguments.argDebugServerBind.c_str()));
        } else {
            m_debugServer.useUnix(arguments.argDebugServerBind);
        }
        m_debugServer.startThread();

        ctx.appendSink(new logging::DebugServerSink(&m_debugServer));

        m_profiler.appendListener(new DebugServerProfiler(&m_debugServer));
    }

    if (!argumentHandler.argProfilingFile.empty()) {
        logger.info() << "Enable profiling";
        m_profiler.appendListener(new FileProfiler(arguments.argProfilingFile));
    } else {
        logger.info() << "Disable profiling";
    }

    logger.info() << "RunLevel " << arguments.argRunLevel;

    std::unique_ptr<logging::ThresholdFilter> filter;

    if (arguments.argEnableLoad) {
        if (arguments.argEnableLoadPath.find('/') == std::string::npos) {
            // no slashes
            m_loadLogPath =
                homepath() + "/.lmslog/" + arguments.argEnableLoadPath;
        } else {
            m_loadLogPath = arguments.argEnableLoadPath;
        }

        if (fileType(m_loadLogPath) != FileType::DIRECTORY) {
            logger.error() << "Given load path is not a directory: "
                           << m_loadLogPath;
            return;
        }

        logger.info() << "Enable load: " << m_loadLogPath;
    }

    const char *lms_config_path = std::getenv("LMS_CONFIG_PATH");
    if (!arguments.configPath.empty()) {
        configPath = arguments.configPath;
    } else if (isEnableLoad()) {
        configPath = loadPath() + "/configs";
    } else if (lms_config_path != nullptr && lms_config_path[0] != '\0') {
        // use LMS_CONFIG_PATH from environment
        configPath = lms_config_path;
    } else {
        // Fallback to built-in config path
        configPath = LMS_CONFIGS;
    }

    if (arguments.argEnableSave) {
        m_saveLogPath = homepath() + "/.lmslog";
        mkdir(m_saveLogPath.c_str(), MODE);

        m_saveLogPath += "/" + currentTimeString();
        if (!arguments.argEnableSaveTag.empty()) {
            m_saveLogPath += "-" + arguments.argEnableSaveTag;
        }
        mkdir(m_saveLogPath.c_str(), MODE);

        copyTree(configPath, m_saveLogPath + "/configs");

        logger.info() << "Enable save: " << m_saveLogPath;
    }

    char *lms_service_path = std::getenv("LMS_SERVICE_PATH");
    if (lms_service_path != nullptr && lms_service_path[0] != '\0') {
        for (auto const &path : split(lms_service_path, ':')) {
            m_serviceLoader.addSearchPath(path, 0);
        }
    }
#ifndef LMS_STANDALONE
//    m_serviceLoader.addSearchPath(LMS_SERVICES, 0);
#endif

    // parse framework config
    if (arguments.argRunLevel >= RunLevel::CONFIG) {

        Runtime *rt = new Runtime("default", *this);
        registerRuntime(rt);

        XmlParser parser(*this, rt, arguments);
        parser.parseConfig(XmlParser::LoadConfigFlag::LOAD_EVERYTHING,
                           arguments.argLoadConfiguration, configPath);

        for (const auto &rt : runtimes) {
            logger.info("registerRuntime")
                << rt.first << " "
                << lms::executionTypeName(rt.second->executionType());
        }

        filter = parser.filter();

        for (auto error : parser.errors()) {
            logger.error() << error;
        }

        for (auto file : parser.files()) {
            configMonitor.watch(file);
        }
    }

    if (arguments.argRunLevel >= RunLevel::ENABLE) {
        for (auto &service : services) {
            if (m_serviceLoader.load(service.second.get())) {
                service.second->instance()->initBase(
                    service.second.get(), service.second->defaultLogLevel());

                try {
                    if (!service.second->instance()->init()) {
                        logger.error() << "Library " << service.first
                                       << " failed to init()";
                        return;
                    }
                } catch (std::exception const &ex) {
                    logger.error() << service.first << " throws "
                                   << lms::typeName(ex) << " : " << ex.what();
                    return;
                }
            } else {
                return;
            }
        }

        char *lms_module_path = std::getenv("LMS_MODULE_PATH");
        if (lms_module_path != nullptr && lms_module_path[0] != '\0') {
            for (auto const &path : split(lms_module_path, ':')) {
                m_moduleLoader.addSearchPath(path, 0);
            }
        }
#ifndef LMS_STANDALONE
//        m_moduleLoader.addSearchPath(LMS_MODULES, 0);
#endif

        // enable modules after they were made available
        logger.info() << "Start enabling modules";

        for (auto &runtime : runtimes) {
            if (!runtime.second->enableModules()) {
                return;
            }
        }
    }

    if (arguments.argRunLevel >= RunLevel::CYCLE) {
        logger.info() << "Start running modules";

        if (!filter) { // check if filter == nullptr
            filter.reset(
                new logging::ThresholdFilter(arguments.argLoggingThreshold));
        }
        if (arguments.argDefinedLoggingThreshold) {
            filter->defaultThreshold(arguments.argLoggingThreshold);
        }
        ctx.filter(filter.release());

        // start threaded runtimes
        for (auto &runtime : runtimes) {
            if (runtime.second->executionType() ==
                ExecutionType::NEVER_MAIN_THREAD) {
                runtime.second->startAsync();
            }
        }

        // run main thread runtimes
        m_running = true;

        while (m_running) {
            bool anyCycle = false;

            for (auto &runtime : runtimes) {
                if (runtime.second->executionType() ==
                    ExecutionType::ONLY_MAIN_THREAD) {
                    if (runtime.second->cycle()) {
                        anyCycle = true;
                    }
                }
            }

            // wait some time if there are no main thread runtimes
            // TODO this should wait for SIGINT instead
            if (!anyCycle) {
                Time::fromMillis(100).sleep();
            }

            // config monitor stuff
            if (configMonitor.hasChangedFiles()) {
                logger.info() << "Reload configs";
                configMonitor.unwatchAll();
                XmlParser parser(*this, getRuntimeByName("default"), arguments);
                parser.parseConfig(
                    XmlParser::LoadConfigFlag::ONLY_MODULE_CONFIG,
                    arguments.argLoadConfiguration, configPath);

                for (auto error : parser.errors()) {
                    logger.error() << error;
                }

                for (auto file : parser.files()) {
                    configMonitor.watch(file);
                }
            }
        }

        // wait for threaded runtimes to stop
        for (auto &rt : runtimes) {
            if (rt.second->executionType() ==
                ExecutionType::NEVER_MAIN_THREAD) {
                rt.second->join();
            }
        }

        ctx.filter(nullptr);
        logger.info() << "Stopped";
    }

    exportGraphs();
}

Framework::~Framework() {
    for (auto &service : services) {
        if (service.second && service.second->instance() != nullptr) {
            try {
                service.second->instance()->destroy();
            } catch (std::exception const &ex) {
                logger.error() << service.first << " throws "
                               << lms::typeName(ex) << " : " << ex.what();
            }
        }
    }

    SignalHandler::getInstance()
        .removeListener(SIGINT, this)
        .removeListener(SIGSEGV, this);
}

void Framework::signal(int s) {
    switch (s) {
    case SIGINT:
        for (auto &runtime : runtimes) {
            runtime.second->stopAsync();
        }
        m_running = false;

        SignalHandler::getInstance().removeListener(SIGINT, this);

        break;
    case SIGSEGV:
        // Segmentation Fault - try to identify what went wrong;
        std::cerr << std::endl
                  << COLOR_RED
                  << "######################################################"
                  << std::endl
                  << "                   Segfault Found                     "
                  << std::endl
                  << "######################################################"
                  << std::endl
                  << COLOR_WHITE;

        // In Case of Segfault while recovering - shutdown.
        SignalHandler::getInstance().removeListener(SIGSEGV, this);

        printStacktrace();

        exit(EXIT_FAILURE);

        break;
    case SIGUSR1:
        printStacktrace();
        break;
    }
}

bool Framework::exportGraphsHelper(bool isExecOrData) {
    std::string gvPath("/tmp/lms.");
    std::string outPath("/tmp/lms.");
    if (isExecOrData) {
        gvPath += "exec";
        outPath += "exec";
    } else {
        gvPath += "data";
        outPath += "data";
    }
    gvPath += ".gv";
    outPath += ".png";

    std::ofstream file(gvPath);

    if (!file) {
        logger.error() << "Failed to open file: " << gvPath;
        return false;
    }

    DotExporter dot(file);
    dot.startDigraph("dag");
    for (auto &rt : runtimes) {
        dot.startSubgraph(rt.first);
        if (isExecOrData) {
            rt.second->executionManager().writeDAG(dot, rt.first);
        } else {
            dumpModuleChannelGraph(
                rt.second->executionManager().getModuleChannelGraph(), dot,
                rt.first);
        }
        dot.endSubgraph();
    }
    dot.endDigraph();
    file.close();

    if (dot.lastError() != DotExporter::Error::OK) {
        logger.error() << "Dot export failed: " << dot.lastError();
        return false;
    }

#ifdef __linux__
    std::string dotCall = "dot -Tpng " + gvPath + " -o " + outPath;
    std::string xdgOpenCall = "xdg-open " + outPath;

    if (0 != system(dotCall.c_str())) {
        logger.error() << "Failed to execute " << dotCall;
        logger.error() << "Check for file permissions and graphviz package";
        return false;
    }

    if (0 != system(xdgOpenCall.c_str())) {
        logger.error() << "Failed to execute " << xdgOpenCall;
        logger.error() << "Are you using gnome?";
        return false;
    }
#elif __APPLE__
    std::string dotCall = "dot -Tpng " + gvPath + " -o " + outPath;
    std::string openCall = "open " + outPath;

    if (0 != system(dotCall.c_str())) {
        logger.error() << "Failed to execute " << dotCall;
        logger.error() << "Check for file permissions and graphviz package";
        return false;
    }

    if (0 != system(openCall.c_str())) {
        logger.error() << "Failed to execute " << openCall;
        logger.error() << "Are you using gnome?";
        return false;
    }
#else
    logger.info() << "dot -Tpng " << gvPath << " > " << outPath;
    logger.info() << "xdg-open " << outPath;
#endif
    return true;
}

void Framework::exportGraphs() {
    if (argumentHandler.argDAG) {
        logger.info() << "Write dot files...";

        exportGraphsHelper(true);
        exportGraphsHelper(false);
    }
}

void Framework::registerRuntime(Runtime *runtime) {
    runtimes.insert(
        std::make_pair(runtime->name(), std::unique_ptr<Runtime>(runtime)));
}

Runtime *Framework::getRuntimeByName(std::string const &name) {
    return runtimes[name].get();
}

bool Framework::hasRuntime(std::string const &name) {
    return runtimes.find(name) != runtimes.end();
}

ArgumentHandler const &Framework::getArgumentHandler() {
    return argumentHandler;
}

Profiler &Framework::profiler() { return m_profiler; }

Loader &Framework::moduleLoader() { return m_moduleLoader; }

std::shared_ptr<ServiceWrapper>
Framework::getServiceWrapper(std::string const &name) {
    return services[name];
}

void Framework::installService(std::shared_ptr<ServiceWrapper> service) {
    auto it = services.find(service->name());

    if (it != services.end()) {
        logger.error("installService") << "Tried to install service "
                                       << service->name()
                                       << " but was already installed";
    } else {
        services[service->name()] = service;
    }
}

void Framework::reloadService(std::shared_ptr<ServiceWrapper> service) {
    std::shared_ptr<ServiceWrapper> originalService = services[service->name()];

    std::unique_lock<std::mutex> lock(originalService->mutex());
    originalService->update(std::move(*service.get()));
    originalService->instance()->configsChanged();
}

bool Framework::isDebug() const { return argumentHandler.argDebug; }

std::string Framework::loadPath() const { return m_loadLogPath; }

std::string Framework::loadLogObject(std::string const &name, bool isDir) {
    if (!isEnableLoad()) {
        throw std::runtime_error(
            "Command line option --enable-load was not specified");
    }

    std::string logobj = m_loadLogPath + "/" + name;

    return isDir ? logobj + "/" : logobj;
}

std::string Framework::saveLogObject(std::string const &name, bool isDir) {
    if (!isEnableSave()) {
        throw std::runtime_error(
            "Command line option --enable-save was not specified");
    }

    std::string logobj = m_saveLogPath + "/" + name;

    if (isDir) {
        mkdir(logobj.c_str(), MODE);
    }

    return isDir ? logobj + "/" : logobj;
}

bool Framework::isEnableLoad() const { return argumentHandler.argEnableLoad; }

bool Framework::isEnableSave() const { return argumentHandler.argEnableSave; }

} // namespace internal
} // namespace lms
