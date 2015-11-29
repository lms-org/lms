#ifndef LMS_MODULE_H
#define LMS_MODULE_H

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <lms/loader.h>
#include <lms/module_config.h>
#include <lms/messaging.h>
#include "lms/data_channel.h"
#include "lms/deprecated.h"
#include "lms/definitions.h"
#include "lms/datamanager.h"
#include "lms/service_handle.h"

namespace lms {

class DataManager;
class ExecutionManager;

#define LMS_MODULE_INTERFACE(CLASS) extern "C" { \
void* getInstance () { \
    return new CLASS(); \
} \
uint32_t getLmsVersion() { \
    return LMS_VERSION_CODE; \
} \
}


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
    typedef ModuleWrapper WrapperType;

    Module(): logger(""), m_datamanager(nullptr),
        m_messaging(nullptr), m_fakeDataManager(this) { }
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
    bool initializeBase(std::shared_ptr<ModuleWrapper> loaderEntry,
                        logging::Level minLogLevel);

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

    /**
     * @brief The FakeDataManager mimics the behavior of the old data manager.
     * This should make it backwards-compatible in most cases.
     *
     * This class is temporary and will be removed in a future release.
     */
    class FakeDataManager {
    public:
        FakeDataManager(Module *module) : module(module) {}

        template<typename T>
        ReadDataChannel<T> readChannel(Module* module, const std::string &name) {
            (void)module;  // thanks for the pointer anyway
            return this->module->readChannel<T>(name);
        }

        template<typename T>
        WriteDataChannel<T> writeChannel(Module* module, const std::string &name) {
            (void)module;  // thanks again
            return this->module->writeChannel<T>(name);
        }

        /**
         * @brief Registers the given module to have write access on
         * a data channel. This will not create the data channel.
         *
         * @param module requesting module
         * @param name data channel name
         */
        DEPRECATED
        void getWriteAccess(Module* module, const std::string &reqName) {
            writeChannel<Any>(module, reqName);
        }

        /**
         * @brief Register the given module to have read access
         * on a data channel. This will not create the data channel.
         *
         * @param module requesting module
         * @param name data channel name
         */
        DEPRECATED
        void getReadAccess(Module* module, const std::string &reqName) {
            readChannel<Any>(module, reqName);
        }

        /**
         * @brief Serialize a data channel into the given output stream.
         *
         * The data channel must have been initialized before you can
         * use this method.
         *
         * A module needs at least read access on the data channel
         * to be able to serialize it.
         *
         * @param module requesting module
         * @param name data channel name
         * @param os output stream to serialize into
         * @return false if the data channel was not initialized or if it
         * is not serializable or if no read or write access, otherwise true
         */
        DEPRECATED
        bool serializeChannel(Module* module, const std::string &reqName, std::ostream &os) {
            return readChannel<Any>(module, reqName).serialize(os);
        }

        /**
         * @brief Deserialize a data channel from the given input stream.
         *
         * The data channel must have been initialized before you use
         * this method.
         *
         * A module needs write access on the data channel to
         * be able to deserialize it.
         *
         * @param module requesting module
         * @param name data channel name
         * @param is input stream to deserialize from
         * @return false if the data channel was not initialized
         * or if it is not serializable or if no write access, otherwise true
         */
        DEPRECATED
        bool deserializeChannel(Module* module, const std::string &reqName, std::istream &is) {
            return writeChannel<Any>(module, reqName).deserialize(is);
        }
    private:
        Module *module;
    };
protected:

    template <class T>
    ServiceHandle<T> getService(std::string const& name) {
        std::shared_ptr<ServiceWrapper> wrapper =
                m_wrapper->getServiceWrapper(name);

        if(wrapper /*&& wrapper->checkHashCode(typeid(T).hash_code())*/) {
            // TODO type check
            return ServiceHandle<T>(wrapper);
        } else {
            // return invalid handle
            return ServiceHandle<T>();
        }
    }

    /**
     * @brief We do not return any data manager pointer any longer. This
     * method is deprecated will be removed in a future release.
     */
    DEPRECATED
    FakeDataManager* datamanager() { return &m_fakeDataManager; }

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
    DEPRECATED
    const ModuleConfig* getConfig(const std::string &name = "default");
    const ModuleConfig& config(const std::string &name = "default");

    bool hasConfig(const std::string &name = "default");

    template<typename T>
    ReadDataChannel<T> readChannel(const std::string &reqName) {
        return m_datamanager->readChannel<T>(m_wrapper, reqName);
    }

    template<typename T>
    WriteDataChannel<T> writeChannel(const std::string &reqName) {
        return m_datamanager->writeChannel<T>(m_wrapper, reqName);
    }
private:
    std::shared_ptr<ModuleWrapper> m_wrapper;
    DataManager* m_datamanager;
    Messaging* m_messaging;
    ExecutionManager* m_executionManager;
    FakeDataManager m_fakeDataManager;
};

}  // namespace lms

#endif /* LMS_MODULE_H */
