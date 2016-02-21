#ifndef LMS_LOADER_H
#define LMS_LOADER_H

#include "lms/logger.h"

namespace lms {
namespace internal {

class Wrapper;

/**
 * @brief Open so/dylib/dll files and load Module
 * or Service objects.
 */
class Loader {
public:
    Loader() : logger("lms.Loader") {
    }

    /**
     * @brief All all so/dylb/dll files in the given directory to the search tree.
     * @param path Directory to search in
     * @param recursion
     */
    void addSearchPath(std::string const& path, int recursion = 0);

    /**
     * @brief Open a so/dylib/dll file and load the wrapper with an instance.
     * @param wrapper Wrapper object to load
     * @return true if successful, false otherwise
     */
    bool load(Wrapper *wrapper);
private:
    /**
     * Cast from one type to a totally other one.
     *
     * We use it here to convert void* to function pointer.
     *
     * Avoid warnings in GCC when using reinterpret_cast:
     * ISO C++ forbids casting between pointer-to-function and pointer-to-object [-Wpedantic]
     */
    template<typename Src, typename Target>
    union UnionHack {
        Src src;
        Target target;

        static_assert(sizeof(Src) == sizeof(Target), "Source and Target must be same size");
    };

    logging::Logger logger;
    std::map<std::string, std::string> m_pathMapping;
};

}  // namespace internal
}  // namespace lms

#endif /* LMS_LOADER_H */
