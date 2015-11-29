#ifndef LMS_SERVICE_WRAPPER_H
#define LMS_SERVICE_WRAPPER_H

#include <string>
#include <mutex>

#include "lms/module_config.h"
#include "lms/service.h"

namespace lms {

class Service;
class Framework;

class ServiceWrapper {
public:
    ServiceWrapper(Framework *runtime);

    std::string name() const;
    void name(std::string const& name);

    std::string libname() const;
    void libname(std::string const& libname);

    Framework* framework();
    Service* instance();
    void instance(Service *service);
    std::mutex& mutex();

    ModuleConfig& getConfig(std::string const& name);
    bool checkHashCode(size_t hashCode);

    void update(ServiceWrapper && other);
private:
    Framework *m_framework;
    std::string m_name;
    std::map<std::string, ModuleConfig> m_configs;
    std::unique_ptr<Service> m_service;
    std::string m_libname;

    std::mutex m_mutex;
};

}  // namespace lms

#endif // LMS_SERVICE_WRAPPER_H
