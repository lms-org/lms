//TODO remove unused imports
#include <climits>
#include <cstring>
#include <cstdio>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <lms/loader.h>
#include <lms/module.h>
#include <lms/logger.h>
namespace lms{
template<typename _Target>
union converter {
    void* src;
    _Target target;
};

std::string Loader::getModulePath(const std::string &libname) {
    return "lib" + libname + ".so";
}

bool Loader::checkSharedLibrary(const std::string &libpath) {
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

Module* Loader::load(module_entry& entry) {
    // for information on dlopen, dlsym, dlerror and dlclose
    // see here: http://linux.die.net/man/3/dlclose

    // open dynamic library (*.so file)
    // http://stackoverflow.com/questions/10765320/compile-c-program-using-dlopen-and-dlsym-with-fpic
    void *lib = dlopen(entry.libpath.c_str(),RTLD_NOW | RTLD_DEEPBIND);

    // check for errors while opening
    if(lib == NULL) {
        logger.error("load") << "Could not open dynamic lib: " << entry.name
            << std::endl << "Message: " << dlerror();
        return nullptr;
    }

    // clear error code
    dlerror();

    // get the pointer to a C-function with name 'getInstance'
    // that was declared inside the dynamic library
    void* func = dlsym(lib, "getInstance");

    // check for errors while calling dlsym
    char *err;
    if ((err = dlerror()) != NULL) {
        logger.error("load") << "Could not get symbol 'getInstance' of module " << entry.name
            << std::endl << "Message: " << err;
        return nullptr;
    }

    entry.dlHandle = lib;

    // TODO check if close is needed here
//    if(dlclose(lib) != 0) {
//        logger.error("load") << "Could not close dynamic lib: " << entry.name
//            << std::endl << "Message: " << dlerror();
//    }

    // Union-Hack to avoid a warning message
    // We use it here to convert a void* to a function pointer.
    // The function has this signature: void* getInstance();
    converter <void*(*)()> conv;
    conv.src = func;

    // call the getInstance function and cast it to a Module pointer
    // -> getInstance should return a newly created object.
    return (Module*)conv.target();


    // Cast symbol to function pointer returning a pointer to a Module instance and
    // call the function to get the a module instance
   /* TODO
     warning: ISO C++ forbids casting between pointer-to-function and pointer-to-object [enabled by default]
     return reinterpret_cast<Module*(*)()>( func )();

    */
    //return reinterpret_cast<Module*(*)()>( func )();
}
}
