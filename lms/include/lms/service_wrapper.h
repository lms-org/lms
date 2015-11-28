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
    ServiceWrapper() {
    }

    std::string name() const;
    void name(std::string const& name);

    Runtime* runtime();
    Service* instance();
    std::mutex& mutex();

    ModuleConfig& getConfig(std::string const& name);
    bool checkHashCode(size_t hashCode);
private:
    Runtime *m_runtime;
    std::string m_name;
    std::map<std::string, ModuleConfig> m_configs;
    std::unique_ptr<Service> m_service;

    std::mutex m_mutex;
};

}  // namespace lms

#endif // LMS_SERVICE_WRAPPER_H
