#ifndef LMS_RUNTIME_H
#define LMS_RUNTIME_H

#include <atomic>

#include "lms/logger.h"
#include "lms/executionmanager.h"
#include "lms/profiler.h"
#include "lms/datamanager.h"
#include "lms/argumenthandler.h"
#include "lms/clock.h"

namespace lms {

class Runtime {
public:
    Runtime(const std::string& name, const ArgumentHandler &args);

    /**
     * @brief Start an infinite cycle loop asynchronously.
     */
    void startAsync();

    /**
     * @brief Stop an infinite cycle loop if previously started by startAsync().
     */
    void stopAsync();

    void join();

    /**
     * @brief Synchronously execute a single cycle of this runtime.
     */
    void cycle();

    void enableModules();

    Profiler& profiler();
    ExecutionManager& executionManager();
    DataManager& dataManager();
    Clock& clock();
    std::string name();
private:
    std::string m_name;
    lms::logging::Logger logger;

    const ArgumentHandler &m_argumentHandler;

    Profiler m_profiler;
    ExecutionManager m_executionManager;
    Clock m_clock;

    /**
     * @brief running just for main-while-loop if it's set to false, the programm will terminate
     */
    std::atomic_bool running;

    std::thread m_thread;
};

}

#endif // LMS_RUNTIME_H
