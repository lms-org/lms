#include "lms/internal/clock.h"

namespace lms {
namespace internal {

Clock::Clock()
    : logger("lms.Clock"), loopTime(Time::ZERO),
      m_enabledSleep(false), m_enabledSlowWarning(false), firstIteration(true),
      overflowTime(Time::ZERO) {
}

void Clock::cycleTime(Time cycleTime) {
    this->loopTime = cycleTime;
}

Time Clock::cycleTime() const {
    return this->loopTime;
}

void Clock::beforeLoopIteration() {
    if(! firstIteration) {
        Time deltaWork = Time::now() - beforeWorkTimestamp;

        if(deltaWork > loopTime && loopTime > lms::Time::ZERO) {
            float ratio = deltaWork.toFloat() / loopTime.toFloat();
            int percentage = int((ratio - 1) * 100);
            logger.warn() << "Cycle was too slow: " << deltaWork << " (+ "
                << percentage << "%)";
        }

        // compute time that should be slept
        Time computedSleep = loopTime - deltaWork - overflowTime;

        if(computedSleep > Time::ZERO) {
            //PrecisionTime beforeSleep = PrecisionTime::now();
            if(m_enabledSleep) {
                computedSleep.sleep();
            }
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

void Clock::enabledSleep(bool flag) {
    m_enabledSleep = flag;
}

bool Clock::enabledSleep() const {
    return m_enabledSleep;
}

void Clock::enabledSlowWarning(bool flag) {
    m_enabledSlowWarning = flag;
}

bool Clock::enabledSlowWarning() const {
    return m_enabledSlowWarning;
}

}  // namespace internal
}  // namespace lms
