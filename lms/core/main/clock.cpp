#include "lms/clock.h"

namespace lms {

Clock::Clock(logging::Logger &rootLogger, extra::PrecisionTime loopTime)
    : logger("Clock", &rootLogger), loopTime(loopTime), firstIteration(true),
    overflowTime(extra::PrecisionTime::ZERO) {
}

void Clock::beforeLoopIteration() {
    using extra::PrecisionTime;

    if(! firstIteration) {
        PrecisionTime deltaWork = PrecisionTime::now() - beforeWorkTimestamp;

        // compute time that should be slept
        PrecisionTime computedSleep = loopTime - deltaWork - overflowTime;

        if(computedSleep > PrecisionTime::ZERO) {
            PrecisionTime beforeSleep = PrecisionTime::now();
            computedSleep.sleep();
            PrecisionTime actualSleep = PrecisionTime::now() - beforeSleep;

            //logger.info("sleep") << "Computed " << computedSleep << " Actual " << actualSleep;
        }

        overflowTime = (PrecisionTime::now() - beforeWorkTimestamp) - loopTime;

        //logger.info("overflow") << " Overflow " << overflowTime;
    }

    firstIteration = false;
    // save time before the main loop's body is executed
    beforeWorkTimestamp = PrecisionTime::now();
}

void Clock::afterLoopIteration() {
    // currently nothing
}

}  // namespace lms
