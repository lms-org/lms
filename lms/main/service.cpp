#include "lms/service.h"
#include "lms/internal/service_wrapper.h"
#include "lms/internal/runtime.h"

namespace lms {

Service::Service() : logger("") {}

void Service::initBase(internal::ServiceWrapper *wrapper, lms::logging::Level minLogLevel) {
    m_wrapper = wrapper;
    logger.name = wrapper->name();
    logger.threshold = minLogLevel;
}

const Config& Service::config(const std::string &name) const{
    return m_wrapper->getConfig(name);
}

std::string Service::getName() const {
    return m_wrapper->name();
}

size_t Service::hashCode() const {
    return typeid(this).hash_code();
}

void Service::logLevelChanged(logging::Level level) {
    logger.threshold = level;
}

}  // namespace lms
