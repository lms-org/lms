#ifndef LMS_SERVICE_H
#define LMS_SERVICE_H

#include <string>

#include "config.h"
#include "logger.h"
#include "lms/definitions.h"
#include "interface.h"

namespace lms {

namespace internal {
class ServiceWrapper;
}

/**
 * @brief Abstract super class for an LMS service. Services are loaded and
 * will be initialized and destroyed by the framework.
 */
class Service {
public:
    Service();

    void initBase(internal::ServiceWrapper *wrapper, lms::logging::Level minLogLevel);

    virtual ~Service() {}

    /**
     * @brief Initialize a service. Must be overridden in subclasses.
     * @return true if the initialization was succesful, otherwise false
     */
    virtual bool init() =0;

    /**
     * @brief Destroy a service. Must be overridden in subclasses.
     */
    virtual void destroy() =0;

    /**
     * @brief This method is called by the framework if any config file has
     * changed. This method may be called after cycle().
     */
    virtual void configsChanged() {}

    /**
     * @brief Return the name of a service. The framework sets a service's name
     * based on its definition in an XML config.
     * @return service name
     */
    std::string getName() const;

    /**
     * @brief TODO This method is currently not rather useful.
     */
    size_t hashCode() const;

    void logLevelChanged(logging::Level level);
protected:
    /**
     * @brief Return a read-only config of the given name.
     * @param name config's name
     * @return module config
     */
    const Config& config(const std::string &name = "default") const;
    logging::Logger logger;
private:
    internal::ServiceWrapper *m_wrapper;
};

}  // namespace lms

#endif // LMS_SERVICE_H
