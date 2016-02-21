#include "lms/internal/service_wrapper.h"

namespace lms {
namespace internal {

ServiceWrapper::ServiceWrapper(Framework *framework) : m_framework(framework),
                                                       m_defaultLevel(logging::Level::ALL) {
}

Config& ServiceWrapper::getConfig(std::string const& name){
    return m_configs[name];
}
const Config& ServiceWrapper::getConfig(std::string const& name) const{
    //It should already exist
    return m_configs.at(name);
}

Framework* ServiceWrapper::framework() {
    return m_framework;
}

Service* ServiceWrapper::instance() {
    return m_service.get();
}

void ServiceWrapper::load(void* instance) {
    m_service.reset(static_cast<Service*>(instance));
}

void ServiceWrapper::unload() {
    m_service.reset();
}

std::mutex& ServiceWrapper::mutex() {
    return m_mutex;
}

bool ServiceWrapper::checkHashCode(size_t hashCode) {
    return m_service && (m_service->hashCode() == hashCode);
}

void ServiceWrapper::update(ServiceWrapper && other) {
    // preserve location of existing ModuleConfigs
    for(auto&& entry : other.m_configs) {
        this->m_configs[entry.first] = entry.second;
    }

    this->m_defaultLevel = other.m_defaultLevel;
    if(m_service) {
        m_service->logLevelChanged(other.m_defaultLevel);
    }
}

void ServiceWrapper::defaultLogLevel(logging::Level level) {
    m_defaultLevel = level;
}

logging::Level ServiceWrapper::defaultLogLevel() const {
    return m_defaultLevel;
}

std::string ServiceWrapper::interfaceFunction() const {
    return "lms_service_" + clazz();
}

}  // namespace internal
}  // namespace lms
