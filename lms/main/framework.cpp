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

    if(argumentHandler.argProfiling) {
        logger.info() << "Enable profiling";
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

        Runtime* rt = new Runtime("default", arguments);
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
            for(auto& runtime : runtimes) {
                if(runtime.second->executionType() == ExecutionType::ONLY_MAIN_THREAD) {
                    runtime.second->cycle();
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

void Framework::exportGraphs() {
    if(! argumentHandler.argDotFile.empty()) {
        std::string dataFile(argumentHandler.argDotFile + ".data.gv");
        std::string execFile(argumentHandler.argDotFile + ".exec.gv");

        std::ofstream dataGraphFile(dataFile);
        std::ofstream execGraphFile(execFile);

        if(! dataGraphFile) {
            logger.error() << "Failed to open file: " << dataFile;
        } else if(! execGraphFile) {
            logger.error() << "Failed to open file: " << execFile;
        } else {
            logger.info() << "Write dot files...";

            lms::extra::DotExporter dotExec(execGraphFile);
            dotExec.startDigraph("exec");
            for(auto& rt : runtimes) {
                dotExec.startSubgraph(rt.first);
                rt.second->executionManager().writeDAG(dotExec, rt.first);
                dotExec.endSubgraph();
            }
            dotExec.endDigraph();
            execGraphFile.close();

            bool successExec = dotExec.lastError() == lms::extra::DotExporter::Error::OK;
            if(! successExec) {
                logger.error() << "Dot export failed: " << dotExec.lastError();
            }

            lms::extra::DotExporter dotData(dataGraphFile);
            dotData.startDigraph("data");
            for(auto& rt : runtimes) {
                dotData.startSubgraph(rt.first);
                rt.second->dataManager().writeDAG(dotData, rt.first);
                dotData.endSubgraph();
            }
            dotData.endDigraph();
            dataGraphFile.close();

            bool successData = dotData.lastError() == lms::extra::DotExporter::Error::OK;
            if(! successData) {
                logger.error() << "Dot export failed: " << dotData.lastError();
            }

            if(successExec && successData) {
                logger.info() << "Execute the following line to create a PNG:";
                logger.info() << "dot -Tpng " << dataFile << " > output.png";
                logger.info() << "xdg-open output.png";
            }
        }
    }
}

void Framework::registerRuntime(Runtime *runtime) {
    runtimes.insert(std::make_pair(runtime->name(), std::unique_ptr<Runtime>(runtime)));
}

Runtime* Framework::getRuntimeByName(std::string const& name) {
    return runtimes[name].get();
}

ArgumentHandler const& Framework::getArgumentHandler() {
    return argumentHandler;
}

}
