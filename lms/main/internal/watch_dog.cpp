#include "lms/internal/watch_dog.h"
#include "lms/internal/backtrace_formatter.h"
#include <pthread.h>
#include <signal.h>

namespace lms{
namespace internal {

// TODO use mutex in beginModule/endModule/thread

WatchDog::WatchDog() : logger("WatchDog"), m_hasModule(false), m_barked(false) {}

void WatchDog::watch(Time maxExecTime) {
    if(m_thread.joinable()) {
        logger.error() << "WatchDog is initialized already";
        return;
    }

    if(maxExecTime > Time::ZERO) {
        m_running = true;
        m_thread = std::thread([this, maxExecTime] () {
            while(m_running) {
                if(m_hasModule && !m_barked && m_moduleBegin.since() > maxExecTime) {
                    m_barked = true;
                    logger.error() << "Module hangs: " << m_currentModule;
                    pthread_kill(m_executionThread, SIGUSR1);
                }
                lms::Time::fromMillis(100).sleep();
            }
        });
    }
}

void WatchDog::beginModule(std::string const& module) {
    m_executionThread = pthread_self();
    m_moduleBegin = Time::now();
    m_currentModule = module;
    m_hasModule = true;
    m_barked = false;
}

void WatchDog::endModule() {
    m_hasModule = false;
}

WatchDog::~WatchDog() {
    m_running = false;
    if(m_thread.joinable()) {
        m_thread.join();
    }
}

}  // namespace internal
}  // namespace lms
