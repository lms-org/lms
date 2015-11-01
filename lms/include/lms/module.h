#ifndef LMS_MODULE_H
#define LMS_MODULE_H

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <lms/loader.h>
#include <lms/module_config.h>
#include <lms/messaging.h>

namespace lms {

class DataManager;
class ExecutionManager;

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
    Module(): logger(""), m_datamanager(nullptr),
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

    std::shared_ptr<ModuleWrapper> wrapper() const {
        return m_wrapper;
    }

    /**
     * @brief Called by the framework itself at module-creation.
     *
     * Do not call this inside a module!
     */
    bool initializeBase(DataManager* datamanager, Messaging *messaging,
                        ExecutionManager* execManager,
        std::shared_ptr<ModuleWrapper> loaderEntry,
                        logging::Level minLogLevel,
                        const std::string &runtimeName);

    ExecutionType getExecutionType() const;

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
     * @brief This method gets called whenever a config changed during execution.
     *
     * A config is usually only changed when the config monitor found changes
     * in the config files.
     *
     * NOTE: This method can be called even if no config files accessed by this
     * module were changed.
     */
    virtual void configsChanged() {}

    /**
     * @brief getDataChannelMapping
     * @param mapFrom
     * @return the mapping for the given key or the key itself if it has no
     * mapping for it
     */
    std::string getChannelMapping(const std::string &mapFrom) const;

    int getChannelPriority(const std::string &name) const;

    int cycleCounter();

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
    logging::Logger logger;

    /**
     * @brief Returns a pointer to the default module-private
     * configuration.
     */
    const ModuleConfig* getConfig(const std::string &name = "default");
    bool hasConfig(const std::string &name = "default");
private:
    std::shared_ptr<ModuleWrapper> m_wrapper;
    DataManager* m_datamanager;
    Messaging* m_messaging;
    ExecutionManager* m_executionManager;
};

}  // namespace lms

#endif /* LMS_MODULE_H */
