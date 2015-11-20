#include <lms/framework.h>
#include <lms/executionmanager.h>
#include <pugixml.hpp>
#include <fstream>
#include <csignal>
#include <map>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "lms/extra/backtrace_formatter.h"
#include "lms/logger.h"
#include "lms/extra/time.h"
#include "lms/module_config.h"
#include "lms/extra/string.h"
#include "lms/xml_parser.h"
#include "lms/extra/colors.h"
#include "lms/definitions.h"
#include "lms/runtime.h"

namespace lms{

std::string Framework::externalDirectory = LMS_EXTERNAL;
std::string Framework::configsDirectory = LMS_CONFIGS;

Framework::Framework(const ArgumentHandler &arguments) :
    logger("lms.Framework"), argumentHandler(arguments),
    configMonitorEnabled(false), m_running(false) {

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

    configMonitorEnabled = argumentHandler.argConfigMonitor;

    if(configMonitorEnabled) {
        logger.info() << "Enable config monitor";
    } else {
        logger.info() << "Disable config monitor";
    }

    logger.info() << "RunLevel " <<  arguments.argRunLevel;

    std::unique_ptr<logging::ThresholdFilter> filter;

    //parse framework config
    if(arguments.argRunLevel >= RunLevel::CONFIG) {
        logger.info() << "MODULES: " << LMS_MODULES;
        logger.info() << "CONFIGS: " << LMS_CONFIGS;

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
            logger.debug("file") << file;
            configMonitor.watch(file);
        }
    }

    if(arguments.argRunLevel >= RunLevel::ENABLE) {
#ifdef __unix__
        m_loader.addModulePath("/usr/lib/lms");
        m_loader.addModulePath("/usr/local/lib/lms");
#endif
        m_loader.addModulePath(LMS_MODULES, 1);

        // enable modules after they were made available
        logger.info() << "Start enabling modules";

        for(auto& runtime : runtimes) {
            runtime.second->enableModules();
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
                    runtime.second->cycle();
                    anyCycle = true;
                }
            }

            // wait some time if there are no main thread runtimes
            // TODO this should wait for SIGINT instead
            if(! anyCycle) {
                lms::extra::PrecisionTime::fromMillis(100).sleep();
            }

            // config monitor stuff
            if(lms::extra::FILE_MONITOR_SUPPORTED && configMonitorEnabled
                    && configMonitor.hasChangedFiles()) {
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

        extra::printStacktrace();

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

    lms::extra::DotExporter dot(file);
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

    if(dot.lastError() != lms::extra::DotExporter::Error::OK) {
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

BufferedDataManager& Framework::bufferedDataManager() {
    return m_bufferedDataManager;
}

Loader& Framework::loader() {
    return m_loader;
}

}
