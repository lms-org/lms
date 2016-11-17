#ifndef LMS_SERVICE_H
#define LMS_SERVICE_H

#include <string>

#include "config.h"
#include "logger.h"
#include "lms/definitions.h"
#include "interface.h"
#include "life_cycle.h"

namespace lms {

namespace internal {
class ServiceInfo;
}

/**
 * @brief Abstract super class for an LMS service. Services are loaded and
 * will be initialized and destroyed by the framework.
 */
class Service : public LifeCycle {
public:
    Service();
    virtual ~Service();

    // Disallow copying
    Service(Service const&) = delete;
    Service& operator=(Service const&) = delete;

    void initBase(const lms::internal::ServiceInfo &info);

    /**
     * @brief Initialize a service. Must be overridden in subclasses.
     * @return true if the initialization was succesful, otherwise false
     */
    virtual bool init() override;

    /**
     * @brief Destroy a service. Must be overridden in subclasses.
     */
    virtual void destroy() override;

    /**
     * @brief This method is called by the framework if any config file has
     * changed. This method may be called after cycle().
     */
    virtual void configsChanged() override;

    /**
     * @brief Return the name of a service. The framework sets a service's name
     * based on its definition in an XML config.
     * @return service name
     */
    std::string getName() const;

    std::mutex& getMutex();
protected:
    /**
     * @brief Return a read-only config of the given name.
     * @param name config's name
     * @return module config
     */
    const Config &config(const std::string &name = "default") const;
    logging::Logger logger;

private:
    struct Private;
    Private *dptr;
    inline Private *dfunc() { return dptr; }
    inline const Private *dfunc() const { return dptr; }
};

} // namespace lms

#endif // LMS_SERVICE_H
