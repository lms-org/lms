#ifndef LMS_SERVICE_WRAPPER_H
#define LMS_SERVICE_WRAPPER_H

#include <string>
#include <mutex>

#include "lms/config.h"
#include "lms/service.h"

namespace lms {
class Service;

namespace internal {

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

    Config& getConfig(std::string const& name);
    const Config& getConfig(std::string const& name) const;
    bool checkHashCode(size_t hashCode);

    void update(ServiceWrapper && other);
private:
    Framework *m_framework;
    std::string m_name;
    std::map<std::string, Config> m_configs;
    std::unique_ptr<Service> m_service;
    std::string m_libname;

    std::mutex m_mutex;
};

}  // namespace internal
}  // namespace lms

#endif // LMS_SERVICE_WRAPPER_H
