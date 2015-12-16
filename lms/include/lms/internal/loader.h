#ifndef LMS_LOADER_H
#define LMS_LOADER_H
/**
  *TODO: use C++ for io
  *
  */
#include <list>
#include <string>
#include <map>

#include "pugixml.hpp"
#include <lms/logger.h>
#include "module_wrapper.h"
#include <lms/logger.h>
#include <lms/module.h>
#include <lms/extra/os.h>
#include <lms/extra/string.h>
#include <climits>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <string>
#include <lms/module.h>
#include <lms/logger.h>

namespace lms {
class Module;

namespace internal {

/**
 * @brief The Loader class used to load Modules
 */
template<class T>
class Loader {
private:
    logging::Logger logger;
    std::map<std::string, std::string> m_pathMapping;
public:
    Loader() : logger("lms.Loader") {
    }

    void addModulePath(std::string const& path, int recursion = 0) {
        std::vector<std::string> list;
        lms::extra::listDir(path, list);

        for(std::string const& child : list) {
            std::string childPath = path + "/" + child;
            lms::extra::FileType type = lms::extra::fileType(childPath);

            if(type == lms::extra::FileType::REGULAR_FILE &&
                    lms::extra::startsWith(child, "lib") &&
                    lms::extra::endsWith(child, ".so")) {
                m_pathMapping[child] = childPath;
            } else if(type == lms::extra::FileType::DIRECTORY && recursion > 0) {
                addModulePath(childPath, recursion - 1);
            }
        }
    }

    template<typename _Target>
    union converter {
        void* src;
        _Target target;
    };

    /**
     * @brief getModulePath
     * @param localPathToModule the path from the modules folder to the module folder that contains the so file
     * @param libname the name of the binary (How it's written in CMakeLists.txt
     * @return the ABSOLUTE path of the shared library file (.so)
     */
    static std::string getModulePath(const std::string &libname) {
        return "lib" + libname + ".so";
    }

    /**
     * @brief checkModule checks if the module could be loaded
     * @param path
     * @return true if module is fine
     */
    bool checkSharedLibrary(const std::string &libpath) {
        void* lib = dlopen (libpath.c_str(), RTLD_LAZY);
        bool valid = false;
        if (lib != NULL) {
            //Testing for Necessary functions
            valid =  (dlsym(lib, "getInstance") != NULL);

            dlclose(lib);
        }else{
            logger.error("checkModule") << "Module doesn't exist! path: " << libpath
                                        << std::endl << dlerror();
        }
        //TODO: not sure if dlclose needed if lib == null
        return valid;
    }

    /**
     * @brief load
     * @return the instance of the module
     */
    bool load(typename T::WrapperType *entry) {
        // for information on dlopen, dlsym, dlerror and dlclose
        // see here: http://linux.die.net/man/3/dlclose

        std::string libpath = m_pathMapping[entry->libname()];

        if(libpath.empty()) {
            logger.error("load") << "libname is empty: " << entry->name();
            return false;
        }

        // open dynamic library (*.so file)
        void *lib = dlopen(libpath.c_str(),RTLD_NOW);

        // check for errors while opening
        if(lib == NULL) {
            logger.error("load") << "Could not open dynamic lib: " << entry->name()
                << std::endl << "Message: " << dlerror();
            return false;
        }

        // clear error code
        dlerror();

        converter <uint32_t (*) ()> getLmsVersion;
        getLmsVersion.src = dlsym(lib, "getLmsVersion");
        char *err;
        if((err = dlerror()) != NULL) {
            logger.warn("load") << "Module " << entry->name() << " does not provide getLmsVersion()";
        } else {
            constexpr uint32_t MAJOR_MASK = LMS_VERSION(0xff, 0, 0);
            constexpr uint32_t MINOR_MASK = LMS_VERSION(0, 0xff, 0);

            uint32_t moduleVersion = getLmsVersion.target();

            if((moduleVersion & MAJOR_MASK) != (LMS_VERSION_CODE & MAJOR_MASK) ||
                    (LMS_VERSION_CODE & MINOR_MASK) < (moduleVersion & MINOR_MASK)) {
                logger.error("load") << "Module " << entry->name() << " has bad version. "
                    << "LMS Version " << LMS_VERSION_STRING << ", Module was compiled for "
                    << lms::extra::versionCodeToString(moduleVersion);
                return false;
            }
        }

        // clear error code
        dlerror();

        // get the pointer to a C-function with name 'getInstance'
        // that was declared inside the dynamic library
        // Union-Hack to avoid a warning message
        // We use it here to convert a void* to a function pointer.
        // The function has this signature: void* getInstance();
        converter <T*(*)()> conv;
        conv.src = dlsym(lib, "getInstance");

        // check for errors while calling dlsym
        if ((err = dlerror()) != NULL) {
            logger.error("load") << "Could not get symbol 'getInstance' of module " << entry->name()
                << std::endl << "Message: " << err;
            return false;
        }

        // TODO check if close is needed here
    //    if(dlclose(lib) != 0) {
    //        logger.error("load") << "Could not close dynamic lib: " << entry.name
    //            << std::endl << "Message: " << dlerror();
    //    }

        // call the getInstance function and cast it to a Module pointer
        // -> getInstance should return a newly created object.
        entry->instance(conv.target());

        // Cast symbol to function pointer returning a pointer to a Module instance and
        // call the function to get the a module instance
       /* TODO
         warning: ISO C++ forbids casting between pointer-to-function and pointer-to-object [enabled by default]
         return reinterpret_cast<Module*(*)()>( func )();

        */
        //return reinterpret_cast<Module*(*)()>( func )();
        return true;
    }

    void unload(typename T::WrapperType *entry) {
        if(entry->enabled()) {
            entry->instance(nullptr);

            // even with dlclose there is a 32 byte memory leak reported by valgrind
            // http://stackoverflow.com/questions/1542457/memory-leak-reported-by-valgrind-in-dlopen

            // DO NOT CLOSE the dynamic lib, it causes segfaults at framework shutdown
    //        if(0 != dlclose(entry.dlHandle)) {
    //            logger.error("unload") << "dlclose failed for " << entry.name;
    //        }
        }
    }
};

}  // namespace internal
}  // namespace lms

#endif /* LMS_LOADER_H */
