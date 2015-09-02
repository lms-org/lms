#ifndef LMS_PROFILER_H
#define LMS_PROFILER_H

#include <vector>
#include <map>
#include <lms/extra/time.h>
#include "lms/logger.h"

namespace lms {

/**
 * @brief Measures module execution time and logs current, min, max and average
 * times in a summary.
 */
class Profiler {
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

    Profiler();

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
    ModuleProfiling &getProfiling(const std::string &module);

    /**
     * @brief Delete all measurement data.
     */
    void resetProfMeasurements();

    /**
     * @brief Add measurement data of a single module.
     * @param profiling module's profiling
     */
    void addProfMeasurement(const ModuleMeasurement &measurement);

    /**
     * @brief Print profiling statistics on the logger
     */
    void printStats();

    /**
     * @brief Enable or disable the profiler.
     */
    void enabled(bool flag);

    /**
     * @brief Returns true if enabled, otherwise false.
     */
    bool enabled() const;

private:
    logging::ChildLogger logger;

    bool m_enabled;
    ProfMeasurements m_profMeasurements;
    ProfilingMap m_profilingMap;
};

}  // namespace lms

#endif /* LMS_PROFILER_H */
