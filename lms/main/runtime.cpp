#include <thread>

#include "lms/runtime.h"

namespace lms {

Runtime::Runtime(const ArgumentHandler &args) :
    logger("Runtime"), m_argumentHandler(args),
    m_executionManager(m_profiler), running(false) {

    m_profiler.enabled(m_argumentHandler.argProfiling);

    m_executionManager.enabledMultithreading(m_argumentHandler.argMultithreaded);

    if(m_argumentHandler.argMultithreaded) {
        if(m_argumentHandler.argThreadsAuto) {
            m_executionManager.numThreadsAuto();
            logger.info() << "Multithreaded with " << m_executionManager.numThreads() << " threads (auto)";
        } else {
            m_executionManager.numThreads(m_argumentHandler.argThreads);
            logger.info() << "Multithreaded with " << m_executionManager.numThreads() << " threads";
        }
    } else {
        logger.info() << "Single threaded";
    }

    if(m_clock.enabled()) {
        logger.info() << "Enabled clock with " << m_clock.cycleTime();
    } else {
        logger.info() << "Disabled clock";
    }
}

Profiler& Runtime::profiler() {
    return m_profiler;
}

ExecutionManager& Runtime::executionManager() {
    return m_executionManager;
}

DataManager& Runtime::dataManager() {
    return m_executionManager.getDataManager();
}

Clock& Runtime::clock() {
    return m_clock;
}

void Runtime::stopAsync() {
    running.store(false);
}

void Runtime::startAsync() {
    running = true;

    m_thread = std::thread([this] () {
        while(running.load()) {
            cycle();
        }
    });
}

void Runtime::join() {
    m_thread.join();
}

void Runtime::cycle() {
    m_clock.beforeLoopIteration();
    if(m_executionManager.profiler().enabled()){
        logger.time("totalTime");
    }
    m_executionManager.loop();
    if(m_executionManager.profiler().enabled()){
        logger.timeEnd("totalTime");
    }

    /*if(lms::extra::FILE_MONITOR_SUPPORTED && configMonitorEnabled
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
    }*/
}

void Runtime::enableModules() {
    m_executionManager.useConfig("default");

    if(m_argumentHandler.argRunLevel == RunLevel::ENABLE) {
        m_executionManager.getDataManager().printMapping();
        m_executionManager.validate();
        m_executionManager.printCycleList();
    }
}

void Runtime::exportGraphs() {
    if(! m_argumentHandler.argDotFile.empty()) {
        std::string dataFile(m_argumentHandler.argDotFile + ".data.gv");
        std::string execFile(m_argumentHandler.argDotFile + ".exec.gv");

        std::ofstream dataGraphFile(dataFile);
        std::ofstream execGraphFile(execFile);

        if(! dataGraphFile) {
            logger.error() << "Failed to open file: " << dataFile;
        } else if(! execGraphFile) {
            logger.error() << "Failed to open file: " << execFile;
        } else {
            logger.info() << "Write dot files...";

            bool success1 = executionManager().getDataManager().writeDAG(dataGraphFile);
            dataGraphFile.close();

            bool success2 = executionManager().writeDAG(execGraphFile);
            execGraphFile.close();

            if(success1 && success2) {
                logger.info() << "Execute the following line to create a PNG:";
                logger.info() << "dot -Tpng " << dataFile << " > output.png";
                logger.info() << "xdg-open output.png";
            }
        }
    }
}

}  // namespace lms
