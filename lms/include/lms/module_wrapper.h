#ifndef LMS_MODULE_WRAPPER_H
#define LMS_MODULE_WRAPPER_H

#include <list>
#include <map>
#include <memory>

#include "lms/extra/time.h"

namespace lms {

class Module;

/**
 * @brief The module_entry struct
 * used to store available modules
 */
struct ModuleWrapper {
    ModuleWrapper() : writePriority(0), enabled(false), moduleInstance(nullptr)
    {}

    /**
     * @brief Name of the module
     */
    std::string name;

    /**
     * @brief full, absolute name of the shared library that shall be loaded
     * if the module is enabled
     */
    std::string libpath;

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
    int writePriority;

    enum ExecutionType {
        ONLY_MAIN_THREAD, NEVER_MAIN_THREAD
    };

    /**
     * @brief The module can only be executed on the specified thread.
     *
     * - ONLY_MAIN_THREAD: The module will only be executed on the main thread.
     * - NEVER_MAIN_THREAD: The module will never be executed on the main thread.
     */
    ExecutionType executionType;

    /**
     * @brief Is set to true if the module is enabled
     */
    bool enabled;

    /**
     * @brief Points to the loaded module instance if enabled, otherwise
     * equals to nullptr
     */
    Module *moduleInstance;

    /**
     * @brief Expected time needed to execute this module.
     *
     * Set to zero if unknown.
     */
    extra::PrecisionTime expectedRuntime;

#ifdef _WIN32
    // TODO pointer to open shared library or something similar
#else
    /**
     * @brief Handle that is returned by dlopen and is needed by dlclose.
     */
    void *dlHandle;
#endif
};

typedef std::list<std::shared_ptr<ModuleWrapper>> ModuleList;

}  // namespace lms

#endif // LMS_MODULE_WRAPPER_H
