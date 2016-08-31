#ifndef _WIN32
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "loader.h"
#include "os.h"
#include "lms/definitions.h"
#include "string.h"

namespace lms {
namespace internal {

bool Loader::exists(const std::string &fileName) {
    std::ifstream infile(fileName);
    return infile.good();
}

void Loader::addSearchPath(std::string const &path) {
    m_paths.push_back(path);
}

Module* Loader::loadModule(const ModuleInfo &info) {
    return static_cast<Module*>(load(info.lib, std::string("lms_module_") + info.clazz));
}

Service* Loader::loadService(const ServiceInfo &info) {
    return static_cast<Service*>(load(info.lib, std::string("lms_service_") + info.clazz));
}

LifeCycle* Loader::load(const std::string &libname, const std::string &function) {
#ifdef _WIN32

    // TODO implementation for Win32
    logger.error("load") << "Not implemented";
    return nullptr;

#else
    // for information on dlopen, dlsym, dlerror and dlclose
    // see here: http://linux.die.net/man/3/dlclose

    bool foundLib = false;
    std::string libpath;

    for(const std::string &path : m_paths) {
        if(exists(libpath = path + "/" + libname + ".so")) {
            foundLib = true;
            break;
        }
#if __APPLE__
        if(exists(libpath = path + "/" + libname + ".dylib")) {
            foundLib = true;
            break;
        }
#endif
    }

    if (!foundLib) {
        LMS_EXCEPTION(std::string("Could not find lib ") + libname);
    }

    // open dynamic library (*.so file)
    void *lib = dlopen(libpath.c_str(), RTLD_NOW);

    // check for errors while opening
    if (lib == NULL) {
        LMS_EXCEPTION(std::string("Could not open dynamic lib ") + libpath + ": " + dlerror());
    }

    // clear error code
    dlerror();

    UnionHack<void *, uint32_t (*)()> getLmsVersion;
    getLmsVersion.src = dlsym(lib, "lms_version");
    char *err;
    if ((err = dlerror()) != NULL) {
        logger.warn("load") << "Lib " << libpath
                            << " does not provide lms_version()";
    } else {
        constexpr uint32_t MAJOR_MASK = LMS_VERSION(0xff, 0, 0);
        constexpr uint32_t MINOR_MASK = LMS_VERSION(0, 0xff, 0);

        uint32_t libVersion = getLmsVersion.target();

        if ((libVersion & MAJOR_MASK) != (LMS_VERSION_CODE & MAJOR_MASK) ||
            (LMS_VERSION_CODE & MINOR_MASK) < (libVersion & MINOR_MASK)) {

            LMS_EXCEPTION(std::string("Lib ") + libpath + " has bad version. "
                          + "LMS Version " + LMS_VERSION_STRING
                          + ", Lib was compiled for " + versionCodeToString(libVersion));
        }
    }

    // clear error code
    dlerror();

    // get the pointer to a C-function with name 'lms_module_*' (or similar)
    // that was declared inside the dynamic library
    // Union-Hack to avoid a warning message
    // We use it here to convert a void* to a function pointer.
    // The function has this signature: void* function_name();
    UnionHack<void *, LifeCycle *(*)()> conv;
    conv.src = dlsym(lib, function.c_str());

    // check for errors while calling dlsym
    if ((err = dlerror()) != NULL) {
        LMS_EXCEPTION(std::string("Could not get symbol ") + function + ": " + err);
    }

    // call the interface function -> should return a newly created object
    return conv.target();
#endif
}

} // namespace internal
} // namespace lms
