#ifndef LMS_RUNTIME_H
#define LMS_RUNTIME_H

#include "lms/logger.h"
#include "executionmanager.h"
#include "profiler.h"
#include "../datamanager.h"
#include "argumenthandler.h"
#include "clock.h"
#include "lms/execution_type.h"

namespace lms {
namespace internal {

class Framework;

class Runtime {
public:
    enum class State {
        RUNNING, PAUSED
    };

    Runtime(const std::string& name, Framework &framework);

    /**
     * @brief Start an infinite cycle loop asynchronously.
     */
    void startAsync();

    /**
     * @brief Stop an infinite cycle loop if previously started by startAsync().
     */
    void stopAsync();

    /**
     * @brief Join the runtime thread (if running).
     */
    void join();

    /**
     * @brief Pause a running runtime.
     */
    void pause();

    /**
     * @brief Resume a pause runtime.
     */
    void resume(bool reset);

    /**
     * @brief Synchronously execute a single cycle of this runtime.
     * @return true if cycle got executed, false if runtime is paused
     */
    bool cycle();

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

    std::thread m_thread;
    ExecutionType m_executionType;

    std::mutex m_mutex;
    State m_state;
    bool m_threadRunning;
    bool m_requestReset;
    std::condition_variable m_cond;
};

}  // namespace internal
}  // namespace lms

#endif // LMS_RUNTIME_H
