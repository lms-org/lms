#ifndef LMS_SERVICE_H
#define LMS_SERVICE_H

#include <string>

#include "lms/module_config.h"
#include "lms/logger.h"
#include "lms/definitions.h"

namespace lms {

#define LMS_SERVICE_INTERFACE(CLASS) extern "C" { \
lms::Service* getInstance () { \
    return new CLASS(); \
} \
uint32_t getLmsVersion() { \
    return LMS_VERSION_CODE; \
} \
}

class ServiceWrapper;

class Service {
public:
    typedef ServiceWrapper WrapperType;

    Service();

    void initBase(ServiceWrapper *wrapper, lms::logging::Level minLogLevel);

    virtual ~Service() {}

    virtual bool init() =0;
    virtual void destroy() =0;

    virtual void configsChanged() {}

    std::string getName() const;
    size_t hashCode() const;
protected:
    const ModuleConfig& config(const std::string &name = "default");
    logging::Logger logger;
private:
    ServiceWrapper *m_wrapper;
};

}  // namespace lms

#endif // LMS_SERVICE_H
