#include "lms/type/framework_info.h"
#include <utility>

namespace lms {
namespace type {

FrameworkInfo::FrameworkInfo() : m_cycleIteration(0), m_moduleProfiling() {}

int FrameworkInfo::cycleIteration() const {
    return m_cycleIteration;
}

void FrameworkInfo::incrementCycleIteration() {
    m_cycleIteration ++;
}

void FrameworkInfo::resetCycleIteration() {
    m_cycleIteration = 0;
}

const FrameworkInfo::Profiling& FrameworkInfo::getProfiling() const {
    return m_moduleProfiling;
}

void FrameworkInfo::resetProfiling() {
    m_moduleProfiling.clear();
}

void FrameworkInfo::addProfilingData(const ModuleMeasurement &profiling) {
    m_moduleProfiling.push_back(profiling);
}

}  // namespace type
}  // namespace lms
