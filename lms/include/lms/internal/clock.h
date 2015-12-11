#ifndef LMS_CLOCK_H
#define LMS_CLOCK_H

#include <array>

#include "lms/time.h"
#include "lms/logger.h"

namespace lms {
namespace internal {

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
    Clock();

    /**
     * @brief Set the clock's loop time for
     * one loop iteration.
     *
     * @param cycleTime microseconds
     */
    void cycleTime(Time cycleTime);

    /**
     * @brief Get the clock's loop time.
     * @return cycle time in microseconds
     */
    Time cycleTime() const;

    /**
     * @brief Enable or disable the clock.
     * @param flag true for enable, false for disable
     */
    void enabled(bool flag);

    /**
     * @brief Returns true if enabled, false otherwise.
     */
    bool enabled() const;

    /**
     * @brief Should be called directly at the beginning
     * of the main loop's body.
     */
    void beforeLoopIteration();
private:
    logging::Logger logger;

    // configuration
    Time loopTime;
    bool m_enabled;

    // current status variables
    bool firstIteration;
    Time beforeWorkTimestamp;
    Time overflowTime;
};

}  // namespace internal
}  // namespace lms

#endif /* LMS_CLOCK_H */
