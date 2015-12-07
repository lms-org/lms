#include <thread>

#include "lms/runtime.h"
#include "lms/framework.h"

namespace lms {

Runtime::Runtime(const std::string &name, Framework& framework) :
    m_name(name), logger(name), m_framework(framework),
    m_argumentHandler(framework.getArgumentHandler()),
    m_profiler(framework.profiler()),
    m_executionManager(m_profiler, *this),
    m_state(State::RUNNING), m_threadRunning(false) {

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

Framework& Runtime::framework() {
    return m_framework;
}

std::string Runtime::name() const {
    return m_name;
}

ExecutionType Runtime::executionType() const {
    return m_executionType;
}

void Runtime::executionType(ExecutionType type) {
    m_executionType = type;
}

void Runtime::stopAsync() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_threadRunning = false;
    }
    m_cond.notify_one();
}

void Runtime::pause() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_state = State::PAUSED;
    }
    m_cond.notify_one();
}

void Runtime::resume() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_state = State::RUNNING;
    }
    m_cond.notify_one();
}

void Runtime::startAsync() {
    std::unique_lock<std::mutex> lock(m_mutex);
    if(! m_threadRunning) {
        m_threadRunning = true;

        m_thread = std::thread([this] () {
            std::unique_lock<std::mutex> lock(m_mutex);
            while(m_threadRunning) {
                m_cond.wait(lock, [this] () {
                    return m_state == State::RUNNING || !m_threadRunning;
                });

                if(! m_threadRunning) {
                    break;
                }

                lock.unlock();
                cycle();
                lock.lock();
            }
        });
    }
}

void Runtime::join() {
    m_thread.join();
}

bool Runtime::cycle() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_state == State::PAUSED) {
            return false;
        }
    }

    m_clock.beforeLoopIteration();
    m_profiler.markBegin(m_name);
    m_executionManager.loop();
    m_profiler.markEnd(m_name);

    // Config monitor
    m_executionManager.updateOrInstall();
    return true;
}

bool Runtime::enableModules() {
    if(! m_executionManager.useConfig("default")) {
        return false;
    }

    if(m_argumentHandler.argRunLevel == RunLevel::ENABLE) {
        m_executionManager.getDataManager().printMapping();
        m_executionManager.validate();
        m_executionManager.printCycleList();
    }

    return true;
}

std::shared_ptr<ServiceWrapper> Runtime::getServiceWrapper(std::string const& name) {
    return m_framework.getServiceWrapper(name);
}

}  // namespace lms
