#ifndef LMS_LOADER_H
#define LMS_LOADER_H

#include "lms/logger.h"
#include "xml_parser.h"
#include "lms/module.h"

namespace lms {
namespace internal {

class Wrapper;

/**
 * @brief Open so/dylib/dll files and load Module
 * or Service objects.
 */
class Loader {
public:
    Loader() : logger("lms.Loader") {}

    /**
     * @brief All all so/dylb/dll files in the given directory to the search
     * tree.
     * @param path Directory to search in
     * @param recursion
     */
    void addSearchPath(std::string const &path);

    Module *loadModule(const ModuleInfo &info);

    Service *loadService(const ServiceInfo &info);

    void registerLib(const std::string &lib);

    /**
     * @brief Calls get_type in all registered libs.
     * @param dm Framework's current data manager instance
     * @param name Channel name
     * @param type Demangled channel type
     * @return initialized channel or nullptr
     */
    std::shared_ptr<lms::DataChannelInternal>
    getChannel(lms::DataManager &dm, const std::string &name,
               const std::string &type);

private:
    /**
     * @brief Open a so/dylib/dll file and load the wrapper with an instance.
     * @param wrapper Wrapper object to load
     * @return true if successful, false otherwise
     */
    LifeCycle *load(const std::string &libname, const std::string &function);

    std::shared_ptr<lms::DataChannelInternal>
    getChannelByLib(const std::string &libpath, lms::DataManager &dm,
                    const std::string &name, const std::string &type);

    bool exists(const std::string &fileName);

    /**
     * Cast from one type to a totally other one.
     *
     * We use it here to convert void* to function pointer.
     *
     * Avoid warnings in GCC when using reinterpret_cast:
     * ISO C++ forbids casting between pointer-to-function and pointer-to-object
     *[-Wpedantic]
     */
    template <typename Src, typename Target> union UnionHack {
        Src src;
        Target target;

        static_assert(sizeof(Src) == sizeof(Target),
                      "Source and Target must be same size");
    };

    logging::Logger logger;
    std::vector<std::string> m_paths;
    std::vector<std::string> m_libs;
};

}  // namespace internal
}  // namespace lms

#endif /* LMS_LOADER_H */
