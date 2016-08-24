#include "lms/service.h"

namespace lms {

Service::Service() : logger("") {}

bool Service::init() { return true; }

void Service::destroy() {}

void Service::configsChanged() {}

void Service::initBase(const lms::internal::ServiceInfo &info) {
    this->info = info;
    logger.name = info.name;
    logger.threshold = info.log;
}

const Config &Service::config(const std::string &name) const {
    return info.configs.at(name);
}

std::string Service::getName() const { return info.name; }

std::mutex& Service::getMutex() {
    return mutex;
}

} // namespace lms
