#include "lms/type/framework_info.h"
#include <utility>

namespace lms {
namespace type {

FrameworkInfo::ProfilingMap FrameworkInfo::m_profilingMap = {};

FrameworkInfo::FrameworkInfo() : m_cycleIteration(0), m_profMeasurements() {}

int FrameworkInfo::cycleIteration() const {
    return m_cycleIteration;
}

void FrameworkInfo::incrementCycleIteration() {
    m_cycleIteration ++;
}

void FrameworkInfo::resetCycleIteration() {
    m_cycleIteration = 0;
}

const FrameworkInfo::ProfMeasurements& FrameworkInfo::getProfMeasurements() const {
    return m_profMeasurements;
}

void FrameworkInfo::resetProfMeasurements() {
    m_profMeasurements.clear();
}

void FrameworkInfo::addProfMeasurement(const ModuleMeasurement &measurement) {
    m_profMeasurements.push_back(measurement);
}

const FrameworkInfo::ProfilingMap& FrameworkInfo::getProfilingMap() {
    return m_profilingMap;
}

FrameworkInfo::ModuleProfiling &FrameworkInfo::getProfiling(std::string module) {
    if(m_profilingMap.find(module) == m_profilingMap.end()) {
        m_profilingMap[module] = *(new FrameworkInfo::ModuleProfiling());
    }
    return m_profilingMap[module];
}



}  // namespace type
}  // namespace lms
