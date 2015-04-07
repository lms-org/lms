#ifndef LMS_CLOCK_H
#define LMS_CLOCK_H

#include <array>

#include "lms/extra/time.h"
#include "lms/logger.h"

namespace lms {

/**
 * @brief Used in the main loop of class Framework this
 * class measures the time of each iterations and
 * ensures a given clock rate by sleeping.
 *
 * Clock will measure:
 * - loop condition
 * - loop body, including the calls to beforeLoopIteration and afterLoopIteration
 * - the call to sleep (usually included in afterLoopIteration)
 */
class Clock {
public:
    /**
     * @brief Create a new clock given the given clock time
     * @param rootLogger the clock will issue warnings if main
     * loop is to slow
     * @param loopTime duration of a single loop iteration
     */
    Clock(logging::Logger &rootLogger, extra::PrecisionTime loopTime);

    /**
     * @brief Should be called directly at the beginning
     * of the main loop's body.
     */
    void beforeLoopIteration();

    /**
     * @brief Should be called at the end of the
     * main loop's body.
     */
    void afterLoopIteration();
private:
    logging::ChildLogger logger;

    // configuration
    extra::PrecisionTime loopTime;

    // current status variables
    bool firstIteration;
    extra::PrecisionTime beforeWorkTimestamp;
    extra::PrecisionTime overflowTime;
};

}  // namespace lms

#endif /* LMS_CLOCK_H */
