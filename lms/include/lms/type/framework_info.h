#ifndef LMS_TYPE_FRAMEWORK_INFO_H
#define LMS_TYPE_FRAMEWORK_INFO_H

#include <vector>
#include <lms/extra/time.h>

namespace lms {
namespace type {

/**
 * @brief Datachannel class that holds information of the current framework
 * state.
 */
class FrameworkInfo {
public:
    struct ModuleMeasurement {
        int thread;
        std::string module;
        lms::extra::PrecisionTime begin;
        lms::extra::PrecisionTime end;
        lms::extra::PrecisionTime expected;
    };

    typedef std::vector<ModuleMeasurement> Profiling;

    FrameworkInfo();

    /**
     * @brief Get the current cycle number that is zeroed on each framework
     * startup. The cycle number is incremented before each cycle.
     *
     * During the initial module enabling zero is returned by this method.
     *
     * @return current cycle number
     */
    int cycleIteration() const;

    /**
     * @brief Increment the number that is returned by cycleIteration().
     */
    void incrementCycleIteration();

    /**
     * @brief Reset the number that is returned by cycleIteration() to zero.
     */
    void resetCycleIteration();

    /**
     * @brief Return module profiling data of the last cycle.
     */
    const Profiling &getProfiling() const;

    /**
     * @brief Delete all profiling data.
     */
    void resetProfiling();

    /**
     * @brief Add profiling data of a single module.
     * @param profiling module's profiling
     */
    void addProfilingData(const ModuleMeasurement &profiling);
private:
    int m_cycleIteration;
    Profiling m_moduleProfiling;
};

}  // namespace type
}  // namespace lms

#endif /* LMS_TYPE_FRAMEWORK_INFO_H */
