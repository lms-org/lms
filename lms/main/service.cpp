#include "lms/service.h"
#include "lms/service_wrapper.h"
#include "lms/runtime.h"

namespace lms {

void Service::initBase(ServiceWrapper *wrapper, lms::logging::Level minLogLevel) {
    m_wrapper = wrapper;
    logger.name = m_wrapper->runtime()->name() + "." + wrapper->name();
    logger.threshold = minLogLevel;
}

const ModuleConfig& Service::config(const std::string &name) {
    return m_wrapper->getConfig(name);
}

std::string Service::getName() const {
    return m_wrapper->name();
}

size_t Service::hashCode() const {
    return typeid(this).hash_code();
}

}  // namespace lms
