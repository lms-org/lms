#ifndef LMS_MODULE_WRAPPER_H
#define LMS_MODULE_WRAPPER_H

#include <list>
#include <map>
#include <memory>

#include "lms/time.h"
#include "lms/execution_type.h"
#include "lms/module_config.h"
#include "service_wrapper.h"

namespace lms {
class Module;

namespace internal {

class Runtime;

/**
 * @brief The module_entry struct
 * used to store available modules
 */
class ModuleWrapper {
    Runtime *m_runtime;

    /**
     * @brief name of the shared library that will be loaded
     */
    std::string m_libname;

    /**
     * @brief Name of the module
     */
    std::string m_name;

    /**
     * @brief Is set to true if the module is enabled
     */
    bool m_enabled;

    /**
     * @brief Points to the loaded module instance if enabled, otherwise
     * equals to nullptr
     */
    std::unique_ptr<Module> m_moduleInstance;
public:
    ModuleWrapper(Runtime *runtime) : m_runtime(runtime), m_enabled(false),
        m_moduleInstance(nullptr) {}

    std::string libname() const;
    void libname(std::string const& libname);

    std::string name() const;
    void name(std::string const& name);

    bool enabled() const;

    Module* instance() const;
    void instance(Module* instance);

    Runtime* runtime() const;
    void runtime(Runtime* runtime);

    /**
     * @brief Used for transparent datachannel mapping. Maps requested
     * datachannels to the real ones.
     */
    std::map<std::string, std::string> channelMapping;

    /**
     * @brief Priority that is used if more than one module is writing
     * into a single datachannel.
     *
     * Modules with higher priority will be executed earlier.
     */
    std::map<std::string, int> channelPriorities;

    int getChannelPriority(const std::string &name) const;

    std::string getChannelMapping(const std::string &mapFrom) const;

    /**
     * @brief The module can only be executed on the specified thread.
     *
     * - ONLY_MAIN_THREAD: The module will only be executed on the main thread.
     * - NEVER_MAIN_THREAD: The module will never be executed on the main thread.
     */
    ExecutionType executionType;

    std::map<std::string, ModuleConfig> configs;

    void update(ModuleWrapper && other);

    std::shared_ptr<ServiceWrapper> getServiceWrapper(std::string const& name);
};

}  // namespace internal
}  // namespace lms

#endif // LMS_MODULE_WRAPPER_H
