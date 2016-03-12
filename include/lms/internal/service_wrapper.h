#ifndef LMS_SERVICE_WRAPPER_H
#define LMS_SERVICE_WRAPPER_H

#include <string>
#include <mutex>

#include "lms/config.h"
#include "lms/service.h"
#include "wrapper.h"

namespace lms {
class Service;

namespace internal {

class Framework;

class ServiceWrapper : public Wrapper {
public:
    ServiceWrapper(Framework *runtime);

    void defaultLogLevel(logging::Level level);
    logging::Level defaultLogLevel() const;

    Framework* framework();
    Service* instance();
    std::mutex& mutex();

    Config& getConfig(std::string const& name);
    const Config& getConfig(std::string const& name) const;
    bool checkHashCode(size_t hashCode);

    void update(ServiceWrapper && other);

    void load(LifeCycle* instance) override;
    void unload() override;
    std::string interfaceFunction() const override;
private:
    Framework *m_framework;
    std::map<std::string, Config> m_configs;
    std::unique_ptr<Service> m_service;
    logging::Level m_defaultLevel;

    std::mutex m_mutex;
};

}  // namespace internal
}  // namespace lms

#endif // LMS_SERVICE_WRAPPER_H
