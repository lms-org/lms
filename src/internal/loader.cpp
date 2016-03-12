#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lms/internal/loader.h"
#include "lms/internal/os.h"
#include "lms/internal/wrapper.h"
#include "lms/definitions.h"
#include "lms/internal/string.h"

namespace lms {
namespace internal {

void Loader::addSearchPath(std::string const& path, int recursion) {
    constexpr size_t LIB_LEN = lenOf("lib");

    std::vector<std::string> list;
    listDir(path, list);

    for(std::string const& child : list) {
        std::string childPath = path + "/" + child;
        FileType type = fileType(childPath);

        if(type == FileType::REGULAR_FILE &&
                startsWith(child, "lib") &&
#ifdef __APPLE__
                // Shared objects may end in .so or .dylib on OS X
                ( endsWith(child, ".so") || endsWith(child, ".dylib") )
#else
                endsWith(child, ".so")
#endif
        ) {
            size_t dotIndex = child.find_last_of('.');

            m_pathMapping[child.substr(LIB_LEN, dotIndex - LIB_LEN)] = childPath;
        } else if(type == FileType::DIRECTORY && recursion > 0) {
            addSearchPath(childPath, recursion - 1);
        }
    }
}

bool Loader::load(Wrapper *wrapper) {
    // for information on dlopen, dlsym, dlerror and dlclose
    // see here: http://linux.die.net/man/3/dlclose

    std::string libpath = m_pathMapping[wrapper->lib()];

    if(libpath.empty()) {
        logger.error("load") << "Could not find " << wrapper->name();
        return false;
    }

    // open dynamic library (*.so file)
    void *lib = dlopen(libpath.c_str(),RTLD_NOW);

    // check for errors while opening
    if(lib == NULL) {
        logger.error("load") << "Could not open dynamic lib: " << wrapper->name();
        logger.error("load") << "Message: " << dlerror();
        return false;
    }

    // clear error code
    dlerror();

    UnionHack <void*, uint32_t (*) ()> getLmsVersion;
    getLmsVersion.src = dlsym(lib, "lms_version");
    char *err;
    if((err = dlerror()) != NULL) {
        logger.warn("load") << "Lib " << wrapper->name() << " does not provide lms_version()";
    } else {
        constexpr uint32_t MAJOR_MASK = LMS_VERSION(0xff, 0, 0);
        constexpr uint32_t MINOR_MASK = LMS_VERSION(0, 0xff, 0);

        uint32_t libVersion = getLmsVersion.target();

        if((libVersion & MAJOR_MASK) != (LMS_VERSION_CODE & MAJOR_MASK) ||
                (LMS_VERSION_CODE & MINOR_MASK) < (libVersion & MINOR_MASK)) {
            logger.error("load") << "Lib " << wrapper->name() << " has bad version. "
                << "LMS Version " << LMS_VERSION_STRING << ", Lib was compiled for "
                << versionCodeToString(libVersion);
            return false;
        }
    }

    // clear error code
    dlerror();

    // get the pointer to a C-function with name 'lms_module_*' (or similar)
    // that was declared inside the dynamic library
    // Union-Hack to avoid a warning message
    // We use it here to convert a void* to a function pointer.
    // The function has this signature: void* function_name();
    UnionHack <void*, LifeCycle*(*)()> conv;
    std::string getterFunc = wrapper->interfaceFunction();
    conv.src = dlsym(lib, getterFunc.c_str());

    // check for errors while calling dlsym
    if ((err = dlerror()) != NULL) {
        logger.error("load") << "Could not get symbol " << getterFunc;
        logger.error("load") << err;
        return false;
    }

    // call the interface function -> should return a newly created object
    wrapper->load(conv.target());

    return true;
}

}  // namespace internal
}  // namespace lms
