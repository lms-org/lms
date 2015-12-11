#include "lms/internal/clock.h"

namespace lms {
namespace internal {

Clock::Clock()
    : logger("lms.Clock"), loopTime(extra::PrecisionTime::ZERO),
      m_enabled(false), firstIteration(true),
      overflowTime(extra::PrecisionTime::ZERO) {
}

void Clock::cycleTime(extra::PrecisionTime cycleTime) {
    this->loopTime = cycleTime;
}

extra::PrecisionTime Clock::cycleTime() const {
    return this->loopTime;
}

void Clock::beforeLoopIteration() {
    if(! enabled()) return;

    using extra::PrecisionTime;

    if(! firstIteration) {
        PrecisionTime deltaWork = PrecisionTime::now() - beforeWorkTimestamp;

        // compute time that should be slept
        PrecisionTime computedSleep = loopTime - deltaWork - overflowTime;

        if(computedSleep > PrecisionTime::ZERO) {
            //PrecisionTime beforeSleep = PrecisionTime::now();
            computedSleep.sleep();
            //PrecisionTime actualSleep = PrecisionTime::now() - beforeSleep;

            //logger.info("sleep") << "Computed " << computedSleep << " Actual " << actualSleep;
        }

        overflowTime = (PrecisionTime::now() - beforeWorkTimestamp) - loopTime;

        //logger.info("overflow") << " Overflow " << overflowTime;
    }

    firstIteration = false;
    // save time before the main loop's body is executed
    beforeWorkTimestamp = PrecisionTime::now();
}

void Clock::enabled(bool flag) {
    m_enabled = flag;
}

bool Clock::enabled() const {
    return m_enabled;
}

}  // namespace internal
}  // namespace lms
