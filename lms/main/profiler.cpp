#include "lms/profiler.h"
#include "lms/logger.h"
#include <utility>

namespace lms {

Profiler::Profiler() : logger("lms.Profiler"),
    m_profMeasurements() {}

const Profiler::ProfMeasurements& Profiler::getProfMeasurements() const {
    return m_profMeasurements;
}

void Profiler::resetProfMeasurements() {
    m_profMeasurements.clear();
}

void Profiler::addProfMeasurement(const ModuleMeasurement &measurement) {
    m_profMeasurements.push_back(measurement);
}

const Profiler::ProfilingMap& Profiler::getProfilingMap() {
    return m_profilingMap;
}

Profiler::ModuleProfiling &Profiler::getProfiling(const std::string& module) {
    return m_profilingMap[module];
}

void Profiler::printStats() {
    for(const Profiler::ModuleMeasurement &m : m_profMeasurements) {
        Profiler::ModuleProfiling &profiling = getProfiling(m.module);
        extra::PrecisionTime runtime = m.end - m.begin;
        // runtime will be rounded to be at least 1 us if lower than 1 us
        if(runtime == extra::PrecisionTime::ZERO) {
            runtime = extra::PrecisionTime::fromMicros(1);
        }
        extra::PrecisionTime expectedRuntime = m.expected;

        // adjust maximum und minimum runtime
        if(profiling.max == extra::PrecisionTime::ZERO
                || profiling.max < runtime) {
            profiling.max = runtime;
        }
        if(profiling.min == extra::PrecisionTime::ZERO
                || profiling.min > runtime) {
            profiling.min = runtime;
        }

        // check if new value should be added to buffer
        if(profiling.measurements % MOVING_AVERAGE_FREQUENCY == 0) {
            // add new value to moving average buffer and adjust buffer index
            profiling.movingAverageBuffer[profiling.bufferIndex] = runtime;
            profiling.bufferIndex = (profiling.bufferIndex + 1) % MOVING_AVERAGE_SIZE;

            // calculate average of all samples in buffer
            lms::extra::PrecisionTime masum = lms::extra::PrecisionTime::ZERO;
            int macount = 0;
            for (int i=0; i< MOVING_AVERAGE_SIZE; i++) {
                // skip elements in case buffer is not full yet
                if(profiling.movingAverageBuffer[i] != lms::extra::PrecisionTime::ZERO) {
                    masum += profiling.movingAverageBuffer[i];
                    macount++;
                }
            }
            profiling.movingAverage = masum/macount;
        }

        // adjust total runtime and number of measurements
        profiling.sum += runtime;
        profiling.measurements++;

        // calculate average runtime
        extra::PrecisionTime avgruntime = profiling.sum/profiling.measurements;


        // print results
        logging::Level logLevel = logging::Level::INFO;

        if(expectedRuntime != extra::PrecisionTime::ZERO) {
            float ratio = float(runtime.micros()) / float(expectedRuntime.micros());
            float growthPercentage = (ratio - 1) * 100;

            if(growthPercentage >= 100) {
                logLevel = logging::Level::ERROR;
            } else if(growthPercentage >= 20) {
                logLevel = logging::Level::WARN;
            }
        }


        logger.log(logLevel, "") << m.thread << " " << m.module << " " << runtime
            << " (min:" << profiling.min << " | avg:" << avgruntime
            << " | max:" << profiling.max << " | MA:" << profiling.movingAverage
            << " | #" << profiling.measurements << ")";
    }
}

void Profiler::enabled(bool flag) {
    m_enabled = flag;
}

bool Profiler::enabled() const {
    return m_enabled;
}

}  // namespace lms
