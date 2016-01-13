#ifndef LMS_INTERNAL_WATCH_DOG_H
#define LMS_INTERNAL_WATCH_DOG_H

#include "lms/time.h"
#include "lms/logger.h"
#include <thread>

namespace lms {
namespace internal {

class WatchDog {
public:
    WatchDog();
    ~WatchDog();

    void watch(lms::Time maxExecTime);
    void beginModule(std::string const& module);
    void endModule();
private:
    logging::Logger logger;

    lms::Time m_moduleBegin;
    std::string m_currentModule;
    bool m_hasModule;
    bool m_barked;

    std::thread m_thread;
    bool m_running;
    pthread_t m_executionThread;
};

}  // namespace internal
}  // namespace lms

#endif // LMS_INTERNAL_WATCH_DOG_H
