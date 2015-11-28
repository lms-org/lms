#ifndef LMS_SERVICE_WRAPPER_H
#define LMS_SERVICE_WRAPPER_H

#include <string>
#include <mutex>

#include "lms/module_config.h"
#include "lms/service.h"

namespace lms {

class Service;
class Runtime;

class ServiceWrapper {
public:
    ServiceWrapper(Runtime *runtime);

    std::string name() const;
    void name(std::string const& name);

    std::string libname() const;
    void libname(std::string const& libname);

    Runtime* runtime();
    Service* instance();
    void instance(Service *service);
    std::mutex& mutex();

    ModuleConfig& getConfig(std::string const& name);
    bool checkHashCode(size_t hashCode);

    void update(ServiceWrapper && other);
private:
    Runtime *m_runtime;
    std::string m_name;
    std::map<std::string, ModuleConfig> m_configs;
    std::unique_ptr<Service> m_service;
    std::string m_libname;

    std::mutex m_mutex;
};

}  // namespace lms

#endif // LMS_SERVICE_WRAPPER_H
