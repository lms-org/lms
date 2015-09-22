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

namespace lms{

std::string Framework::externalDirectory = LMS_EXTERNAL;
std::string Framework::configsDirectory = LMS_CONFIGS;

Framework::Framework(const ArgumentHandler &arguments) :
    logger("lms.Framework"), argumentHandler(arguments), m_executionManager(),
    configMonitorEnabled(false) {

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

    m_executionManager.profiler().enabled(argumentHandler.argProfiling);

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

    m_executionManager.enabledMultithreading(argumentHandler.argMultithreaded);

    if(argumentHandler.argMultithreaded) {
        if(argumentHandler.argThreadsAuto) {
            m_executionManager.numThreadsAuto();
            logger.info() << "Multithreaded with " << m_executionManager.numThreads() << " threads (auto)";
        } else {
            m_executionManager.numThreads(argumentHandler.argThreads);
            logger.info() << "Multithreaded with " << m_executionManager.numThreads() << " threads";
        }
    } else {
        logger.info() << "Single threaded";
    }

    logger.info() << "RunLevel " <<  arguments.argRunLevel;

    std::unique_ptr<logging::ThresholdFilter> filter;

    //parse framework config
    if(arguments.argRunLevel >= RunLevel::CONFIG) {
        logger.info() << "MODULES: " << LMS_MODULES;
        logger.info() << "CONFIGS: " << LMS_CONFIGS;

        XmlParser parser(*this, arguments);
        parser.parseConfig(XmlParser::LoadConfigFlag::LOAD_EVERYTHING, arguments.argLoadConfiguration);
        filter = parser.filter();

        for(auto error : parser.errors()) {
            logger.error() << error;
        }

        for(auto file : parser.files()) {
            logger.debug("file") << file;
            configMonitor.watch(file);
        }

        if(m_clock.enabled()) {
            logger.info() << "Enabled clock with " << m_clock.cycleTime();
        } else {
            logger.info() << "Disabled clock";
        }
    }

    if(arguments.argRunLevel >= RunLevel::ENABLE) {
        // enable modules after they were made available
        logger.info() << "Start enabling modules";
        m_executionManager.useConfig("default");

        if(arguments.argRunLevel == RunLevel::ENABLE) {
            m_executionManager.getDataManager().printMapping();
            m_executionManager.validate();
            m_executionManager.printCycleList();
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

        //Execution
        running = true;

        while(running) {
            m_clock.beforeLoopIteration();
            if(m_executionManager.profiler().enabled()){
                logger.time("totalTime");
            }
            m_executionManager.loop();
            if(m_executionManager.profiler().enabled()){
                logger.timeEnd("totalTime");
            }

            if(lms::extra::FILE_MONITOR_SUPPORTED && configMonitorEnabled
                    && configMonitor.hasChangedFiles()) {
                configMonitor.unwatchAll();
                XmlParser parser(*this, arguments);
                parser.parseConfig(XmlParser::LoadConfigFlag::ONLY_MODULE_CONFIG,
                                   arguments.argLoadConfiguration);

                for(auto error : parser.errors()) {
                    logger.error() << error;
                }

                for(auto file : parser.files()) {
                    configMonitor.watch(file);
                }

                m_executionManager.fireConfigsChangedEvent();
            }
        }

        ctx.filter(nullptr);
        logger.info() << "Stopped";
    }
}

ExecutionManager& Framework::executionManager() {
    return m_executionManager;
}

Clock& Framework::clock() {
    return m_clock;
}

Framework::~Framework() {
    SignalHandler::getInstance()
            .removeListener(SIGINT, this)
            .removeListener(SIGSEGV, this);
}

void Framework::signal(int s) {
    switch (s) {
    case SIGINT:
        running = false;

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
}
