#ifndef LMS_TYPE_FRAMEWORK_INFO_H
#define LMS_TYPE_FRAMEWORK_INFO_H

#include <vector>
#include <lms/extra/time.h>

namespace lms {
namespace type {

class FrameworkInfo {
public:
    struct ModuleMeasurement {
        std::string module;
        lms::extra::PrecisionTime begin;
        lms::extra::PrecisionTime end;
    };

    typedef std::vector<ModuleMeasurement> Profiling;

    FrameworkInfo();

    int cycleIteration() const;
    void incrementCycleIteration();
    void resetCycleIteration();

    const Profiling &getProfiling() const;
    void resetProfiling();
    void addProfilingData(const ModuleMeasurement &profiling);
private:
    int m_cycleIteration;
    Profiling m_moduleProfiling;
};

}  // namespace type
}  // namespace lms

#endif /* LMS_TYPE_FRAMEWORK_INFO_H */
