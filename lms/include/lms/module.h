#ifndef LMS_MODULE_H
#define LMS_MODULE_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <lms/loader.h>
#include <lms/type/module_config.h>
#include <lms/messaging.h>

namespace lms {

class DataManager;

/**
 * @brief Abstract super class for dynamic loadable modules
 * with a guaranteed lifecycle.
 *
 * Lifecycle:
 * - initialize() - called once after module loading
 * - cycle() - can be called any number of times
 * - deinitialize() - called once before module unloading
 */
class Module {
public:
    Module(): logger("", nullptr), m_datamanager(nullptr),
        m_messaging(nullptr) { }
    virtual ~Module() { }
	
    /**
     * @brief Name of the module itself.
     *
     * Can be called inside a module's initialize, cycle or
     * deinitialize method.
     *
     * @return name of the loaded module
     */
    std::string getName() const;

    /**
     * @brief Called by the framework itself at module-creation.
     *
     * Do not call this inside a module!
     */
    bool initializeBase(DataManager* datamanager, Messaging *messaging,
                        Loader::module_entry& loaderEntry, logging::Logger *rootLogger,
                        logging::LogLevel minLogLevel);

    int getPriority() const;

    /**
     * @brief Informs a module of the start of its
     * lifecycle.
     *
     * Modules should request configs and data channels during
     * execution of this method. Modules should also
     * also allocate memory and open files here.
     *
     * This method will be called only once and before
     * every other lifecycle method.
     *
     * Must be overridden by modules.
     *
     * Returning false will prohibit the module from being
     * loaded. cycle and deinitialize will never be called in
     * this case.
     *
     * @return true if initialization was succesful, otherwise false
     */
    virtual bool initialize() = 0;

    /**
     * @brief Informs a module that it should now read or write to
     * data channels and/or start computing something.
     *
     * Modules can read from or write to data channels.
     *
     * The cycle method can be called never, once or for any other
     * random number of times. The cycle method is guaranteed to be called
     * after initialize and before deinitialize was called.
     *
     * Must be overridden by modules.
     *
     * @return true if cycling was succesul was succesful, otherwise false
     */
    virtual bool cycle() = 0;

    /**
     * @brief Informs a module of the end of its
     * lifecycle.
     *
     * Modules should destroy any allocated memory, file descriptors
     * and call delete on any pointers that were created with
     * new in initialize.
     *
     * This method will be called only once and after
     * every other lifecycle method. This method is
     * guaranteed to be called after initialize() returned
     * true. This method will be never called before initialize().
     *
     * Must be overridden by a module.
     *
     * @return true if deinitialization was succesful, otherwise false
     */
    virtual bool deinitialize() = 0;

    /**
     * @brief reset called by the framework
     * The following implementation is fully backwards compatible:
     * Modules can override reset if they can implement it
     * Don't forget to override isResettable() if you have a reset method!
     */
    virtual void reset() {}

    /**
     * @brief If the reset method is implemented inside a module.
     *
     * If a module overrides reset() then isResettable() should be overridden to return true.
     *
     * @return true if reset() can be called by the framework, otherwise false
     */
    virtual bool isResettable() { return false; }

    /**
     * @brief getDataChannelMapping
     * @param mapFrom
     * @return the mapping for the given key or the key itself if it has no
     * mapping for it
     */
    std::string getChannelMapping(std::string mapFrom);

protected:
    /**
     * @brief Returns the data manager. This is usually
     * used in initialize().
     */
    DataManager* datamanager() const { return m_datamanager; }

    /**
     * @brief Returns the messaging service.
     *
     * Send or receive messages between different modules
     * and the core framework.
     */
    Messaging* messaging() const { return m_messaging; }

    /**
     * @brief A logger instance. Can be used in initialize, cycle
     * and deinitialize to log debugging information.
     */
    logging::ChildLogger logger;

    /**
     * @brief Returns a pointer to the default module-private
     * configuration.
     */
    const type::ModuleConfig* getConfig(const std::string &name = "default");
private:
    Loader::module_entry loaderEntry;
    DataManager* m_datamanager;
    Messaging* m_messaging;
};

}  // namespace lms

#endif /* LMS_MODULE_H */
