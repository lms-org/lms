#ifndef LMS_TYPE_FRAMEWORK_INFO_H
#define LMS_TYPE_FRAMEWORK_INFO_H

#include <vector>
#include <map>
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

    // how many runtime values should be considered for the MA computation (buffer size)
    const static int MOVING_AVERAGE_SIZE = 20;

    // how often are values for the MA computation sampled
    // e.g. 10 = take every 10th value
    const static int MOVING_AVERAGE_FREQUENCY = 10;

    struct ModuleProfiling {
        lms::extra::PrecisionTime current = extra::PrecisionTime::ZERO;
        lms::extra::PrecisionTime min = extra::PrecisionTime::ZERO;
        lms::extra::PrecisionTime max = extra::PrecisionTime::ZERO;
        lms::extra::PrecisionTime sum = extra::PrecisionTime::ZERO;
        int measurements = 0;
        lms::extra::PrecisionTime movingAverage = extra::PrecisionTime::ZERO;
        lms::extra::PrecisionTime movingAverageBuffer[MOVING_AVERAGE_SIZE];
        int bufferIndex = 0;
    };

    typedef std::vector<ModuleMeasurement> ProfMeasurements;
    typedef std::map<std::string, ModuleProfiling> ProfilingMap;

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
     * @brief Return module measurement data of the last cycle.
     */
    const ProfMeasurements &getProfMeasurements() const;

    /**
     * @brief Return profiling map
     */
    const ProfilingMap &getProfilingMap();

    /**
     * @brief Return profiling struct of specific module. Create module profiling
     * object if necessary
     * @param module name
     */
    ModuleProfiling &getProfiling(std::string module);

    /**
     * @brief Delete all measurement data.
     */
    void resetProfMeasurements();

    /**
     * @brief Add measurement data of a single module.
     * @param profiling module's profiling
     */
    void addProfMeasurement(const ModuleMeasurement &measurement);

private:
    int m_cycleIteration;
    ProfMeasurements m_profMeasurements;
    static ProfilingMap m_profilingMap;
};

}  // namespace type
}  // namespace lms

#endif /* LMS_TYPE_FRAMEWORK_INFO_H */
