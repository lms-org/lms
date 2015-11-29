#ifndef LMS_RUNTIME_H
#define LMS_RUNTIME_H

#include "lms/logger.h"
#include "lms/executionmanager.h"
#include "lms/profiler.h"
#include "lms/datamanager.h"
#include "lms/argumenthandler.h"
#include "lms/clock.h"
#include "lms/execution_type.h"

namespace lms {

class Framework;

class Runtime {
public:
    Runtime(const std::string& name, Framework &framework);

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

    bool enableModules();

    Profiler& profiler();
    ExecutionManager& executionManager();
    DataManager& dataManager();
    Clock& clock();
    Framework& framework();
    std::string name() const;
    ExecutionType executionType() const;
    void executionType(ExecutionType type);
    std::shared_ptr<ServiceWrapper> getServiceWrapper(std::string const& name);
private:
    std::string m_name;
    lms::logging::Logger logger;

    Framework &m_framework;
    const ArgumentHandler &m_argumentHandler;

    Profiler &m_profiler;
    ExecutionManager m_executionManager;
    Clock m_clock;

    /**
     * @brief running just for main-while-loop if it's set to false, the programm will terminate
     */
    bool m_running;

    std::thread m_thread;
    ExecutionType m_executionType;
};

}

#endif // LMS_RUNTIME_H
