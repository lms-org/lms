#include <lms/service.h>
#include "internal/xml_parser.h"

namespace lms {

struct Service::Private {
    std::mutex mutex;
    lms::internal::ServiceInfo info;
};

Service::Service() : logger(""), dptr(new Private) {}

Service::~Service() { delete dptr; }

bool Service::init() { return true; }

void Service::destroy() {}

void Service::configsChanged() {}

void Service::initBase(const lms::internal::ServiceInfo &info) {
    dfunc()->info = info;
    logger.name = info.name;
    logger.threshold = info.log;
}

const Config &Service::config(const std::string &name) const {
    return dfunc()->info.configs.at(name);
}

std::string Service::getName() const { return dfunc()->info.name; }

std::mutex& Service::getMutex() {
    return dfunc()->mutex;
}

} // namespace lms
