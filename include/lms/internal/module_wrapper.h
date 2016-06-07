#ifndef LMS_MODULE_WRAPPER_H
#define LMS_MODULE_WRAPPER_H

#include <list>
#include <map>
#include <memory>

#include "lms/execution_type.h"
#include "lms/config.h"
#include "service_wrapper.h"
#include "wrapper.h"

namespace lms {
class Module;

namespace internal {

class Runtime;

/**
 * @brief The module_entry struct
 * used to store available modules
 */
class ModuleWrapper : public Wrapper {
    Runtime *m_runtime;

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
    ModuleWrapper(Runtime *runtime)
        : m_runtime(runtime), m_enabled(false), m_moduleInstance(nullptr) {}

    bool enabled() const;

    Module *instance() const;

    void load(LifeCycle *instance) override;
    void unload() override;

    Runtime *runtime() const;
    void runtime(Runtime *runtime);

    /**
     * @brief Used for transparent datachannel mapping. Maps requested
     * datachannels to the real ones.
     *
     * Priority that is used if more than one module is writing
     * into a single datachannel.
     *
     * Modules with higher priority will be executed earlier.
     */
    std::map<std::string, std::pair<std::string, int>> channelMapping;

    std::pair<std::string, int> mapChannel(const std::string &name) const;

    /**
     * @brief The module can only be executed on the specified thread.
     *
     * - ONLY_MAIN_THREAD: The module will only be executed on the main thread.
     * - NEVER_MAIN_THREAD: The module will never be executed on the main
     *thread.
     */
    ExecutionType executionType;

    std::map<std::string, Config> configs;

    void update(ModuleWrapper &&other);

    std::shared_ptr<ServiceWrapper> getServiceWrapper(std::string const &name);

    std::string interfaceFunction() const override;
};

} // namespace internal
} // namespace lms

#endif // LMS_MODULE_WRAPPER_H
