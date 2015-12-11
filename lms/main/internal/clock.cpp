#include "lms/internal/clock.h"

namespace lms {
namespace internal {

Clock::Clock()
    : logger("lms.Clock"), loopTime(Time::ZERO),
      m_enabled(false), firstIteration(true),
      overflowTime(Time::ZERO) {
}

void Clock::cycleTime(Time cycleTime) {
    this->loopTime = cycleTime;
}

Time Clock::cycleTime() const {
    return this->loopTime;
}

void Clock::beforeLoopIteration() {
    if(! enabled()) return;

    if(! firstIteration) {
        Time deltaWork = Time::now() - beforeWorkTimestamp;

        // compute time that should be slept
        Time computedSleep = loopTime - deltaWork - overflowTime;

        if(computedSleep > Time::ZERO) {
            //PrecisionTime beforeSleep = PrecisionTime::now();
            computedSleep.sleep();
            //PrecisionTime actualSleep = PrecisionTime::now() - beforeSleep;

            //logger.info("sleep") << "Computed " << computedSleep << " Actual " << actualSleep;
        }

        overflowTime = (Time::now() - beforeWorkTimestamp) - loopTime;

        //logger.info("overflow") << " Overflow " << overflowTime;
    }

    firstIteration = false;
    // save time before the main loop's body is executed
    beforeWorkTimestamp = Time::now();
}

void Clock::enabled(bool flag) {
    m_enabled = flag;
}

bool Clock::enabled() const {
    return m_enabled;
}

}  // namespace internal
}  // namespace lms
