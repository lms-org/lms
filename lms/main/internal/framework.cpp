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
#include "lms/internal/backtrace_formatter.h"
#include "lms/logger.h"
#include "lms/time.h"
#include "lms/config.h"
#include "lms/extra/string.h"
#include "lms/internal/xml_parser.h"
#include "lms/extra/colors.h"
#include "lms/definitions.h"
#include "lms/internal/runtime.h"

namespace lms {
namespace internal {

std::string Framework::externalDirectory = LMS_EXTERNAL;
std::string Framework::configsDirectory = LMS_CONFIGS;

Framework::Framework(const ArgumentHandler &arguments) :
    logger("lms.Framework"), argumentHandler(arguments),
    m_running(false) {

    logging::Context &ctx = logging::Context::getDefault();

    if(! arguments.argQuiet) {
        ctx.appendSink(new logging::ConsoleSink(std::cout));
    }

    if(! arguments.argLogFile.empty()) {
        ctx.appendSink(new logging::FileSink(arguments.argLogFile));
    }

    SignalHandler::getInstance()
            .addListener(SIGINT, this)
            .addListener(SIGSEGV, this);

    if(! argumentHandler.argProfilingFile.empty()) {
        logger.info() << "Enable profiling";
        m_profiler.enable(arguments.argProfilingFile);
    } else {
        logger.info() << "Disable profiling";
    }

    logger.info() << "RunLevel " <<  arguments.argRunLevel;

    std::unique_ptr<logging::ThresholdFilter> filter;

#ifndef LMS_STANDALONE
    m_serviceLoader.addModulePath(LMS_SERVICES, 0);
#endif
    char *lms_service_path = std::getenv("LMS_SERVICE_PATH");
    if(lms_service_path != nullptr && lms_service_path[0] != '\0') {
        m_serviceLoader.addModulePath(lms_service_path, 0);
    }

    //parse framework config
    if(arguments.argRunLevel >= RunLevel::CONFIG) {

        Runtime* rt = new Runtime("default", *this);
        registerRuntime(rt);

        XmlParser parser(*this, rt, arguments);
        parser.parseConfig(XmlParser::LoadConfigFlag::LOAD_EVERYTHING, arguments.argLoadConfiguration);

        for(const auto& rt : runtimes) {
            logger.info("registerRuntime") << rt.first << " " <<
                lms::executionTypeName(rt.second->executionType());
        }

        filter = parser.filter();

        for(auto error : parser.errors()) {
            logger.error() << error;
        }

        for(auto file : parser.files()) {
            configMonitor.watch(file);
        }
    }

    if(arguments.argRunLevel >= RunLevel::ENABLE) {
        for(auto& service : services) {
            if(! service.second->instance()->init()) {
                logger.error() << "Library " << service.first << " failed to init()";
                return;
            }
        }

#ifndef LMS_STANDALONE
        m_moduleLoader.addModulePath(LMS_MODULES, 0);
#endif
        char *lms_module_path = std::getenv("LMS_MODULE_PATH");
        if(lms_module_path != nullptr && lms_module_path[0] != '\0') {
            m_moduleLoader.addModulePath(lms_module_path, 0);
        }

        // enable modules after they were made available
        logger.info() << "Start enabling modules";

        for(auto& runtime : runtimes) {
            if(! runtime.second->enableModules()) {
                return;
            }
        }
    }

    if(arguments.argRunLevel >= RunLevel::CYCLE) {
        logger.info() << "Start running modules";

        if(! filter) { // check if filter == nullptr
            filter.reset(new logging::ThresholdFilter(arguments.argLoggingThreshold));
        }
        if(arguments.argDefinedLoggingThreshold) {
            filter->defaultThreshold(arguments.argLoggingThreshold);
        }
        ctx.filter(filter.release());

        // start threaded runtimes
        for(auto& runtime : runtimes) {
            if(runtime.second->executionType() == ExecutionType::NEVER_MAIN_THREAD) {
                runtime.second->startAsync();
            }
        }

        // run main thread runtimes
        m_running = true;

        while(m_running) {
            bool anyCycle = false;

            for(auto& runtime : runtimes) {
                if(runtime.second->executionType() == ExecutionType::ONLY_MAIN_THREAD) {
                    if(runtime.second->cycle()) {
                        anyCycle = true;
                    }
                }
            }

            // wait some time if there are no main thread runtimes
            // TODO this should wait for SIGINT instead
            if(! anyCycle) {
                Time::fromMillis(100).sleep();
            }

            // config monitor stuff
            if(lms::extra::FILE_MONITOR_SUPPORTED && configMonitor.hasChangedFiles()) {
                logger.info() << "Reload configs";
                configMonitor.unwatchAll();
                XmlParser parser(*this, getRuntimeByName("default"), arguments);
                parser.parseConfig(XmlParser::LoadConfigFlag::ONLY_MODULE_CONFIG,
                                   arguments.argLoadConfiguration);

                for(auto error : parser.errors()) {
                    logger.error() << error;
                }

                for(auto file : parser.files()) {
                    configMonitor.watch(file);
                }
            }
        }

        // wait for threaded runtimes to stop
        for(auto& rt : runtimes) {
            if(rt.second->executionType() == ExecutionType::NEVER_MAIN_THREAD) {
                rt.second->join();
            }
        }

        ctx.filter(nullptr);
        logger.info() << "Stopped";
    }

    exportGraphs();
}

Framework::~Framework() {
    for(auto& service : services) {
        service.second->instance()->destroy();
    }

    SignalHandler::getInstance()
            .removeListener(SIGINT, this)
            .removeListener(SIGSEGV, this);
}

void Framework::signal(int s) {
    switch (s) {
    case SIGINT:
        for(auto& runtime : runtimes) {
            runtime.second->stopAsync();
        }
        m_running = false;

        SignalHandler::getInstance().removeListener(SIGINT, this);

        break;
    case SIGSEGV:
        //Segmentation Fault - try to identify what went wrong;
        std::cerr << std::endl << lms::extra::COLOR_RED
                << "######################################################" << std::endl
                << "                   Segfault Found                     " << std::endl
                << "######################################################" << std::endl
                << lms::extra::COLOR_WHITE;

        //In Case of Segfault while recovering - shutdown.
        SignalHandler::getInstance().removeListener(SIGSEGV, this);

        printStacktrace();

        exit(EXIT_FAILURE);

        break;
    }
}

bool Framework::exportGraphsHelper(std::string const& path, bool isExecOrData) {
    std::string gvPath = path;
    std::string outPath = path;
    if(isExecOrData) {
        gvPath += ".exec.gv";
        outPath += ".exec.png";
    } else {
        gvPath += ".data.gv";
        outPath += ".data.png";
    }

    std::ofstream file(gvPath);

    if(! file) {
        logger.error() << "Failed to open file: " << gvPath;
        return false;
    }

    DotExporter dot(file);
    dot.startDigraph("exec");
    for(auto& rt : runtimes) {
        dot.startSubgraph(rt.first);
        if(isExecOrData) {
            rt.second->executionManager().writeDAG(dot, rt.first);
        } else {
            rt.second->dataManager().writeDAG(dot, rt.first);
        }
        dot.endSubgraph();
    }
    dot.endDigraph();
    file.close();

    if(dot.lastError() != DotExporter::Error::OK) {
        logger.error() << "Dot export failed: " << dot.lastError();
        return false;
    }

    logger.info() << "dot -Tpng " << gvPath << " > " << outPath;
    logger.info() << "xdg-open " << outPath;

    return true;
}

void Framework::exportGraphs() {
    if(! argumentHandler.argDotFile.empty()) {
        logger.info() << "Write dot files...";

        exportGraphsHelper(argumentHandler.argDotFile, true);
        exportGraphsHelper(argumentHandler.argDotFile, false);
    }
}

void Framework::registerRuntime(Runtime *runtime) {
    runtimes.insert(std::make_pair(runtime->name(), std::unique_ptr<Runtime>(runtime)));
}

Runtime* Framework::getRuntimeByName(std::string const& name) {
    return runtimes[name].get();
}

bool Framework::hasRuntime(std::string const& name) {
    return runtimes.find(name) != runtimes.end();
}

ArgumentHandler const& Framework::getArgumentHandler() {
    return argumentHandler;
}

Profiler& Framework::profiler() {
    return m_profiler;
}

Loader<Module>& Framework::moduleLoader() {
    return m_moduleLoader;
}

std::shared_ptr<ServiceWrapper> Framework::getServiceWrapper(std::string const& name) {
    return services[name];
}

void Framework::installService(std::shared_ptr<ServiceWrapper> service) {
    auto it = services.find(service->name());

    if(it != services.end()) {
        logger.error("installService") << "Tried to install service "
            << service->name() << " but was already installed";
    } else {
        services[service->name()] = service;
        m_serviceLoader.load(service.get());
        service->instance()->initBase(service.get(), logging::Level::DEBUG);
    }
}

void Framework::reloadService(std::shared_ptr<ServiceWrapper> service) {
    std::shared_ptr<ServiceWrapper> originalService = services[service->name()];

    std::unique_lock<std::mutex> lock(originalService->mutex());
    originalService->update(std::move(*service.get()));
    originalService->instance()->configsChanged();
}

bool Framework::isDebug() const {
    return argumentHandler.argDebug;
}

}  // namespace internal
}  // namespace lms
