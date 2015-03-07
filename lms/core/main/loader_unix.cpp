//TODO remove unused imports
#include <limits.h>
#include <cstring>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <core/loader.h>
#include <core/module.h>
#include <core/logger.h>
namespace lms{
template<typename _Target>
union converter {
    void* src;
    _Target target;
};

bool Loader::checkModule(const char* path){
    void* lib = dlopen (path, RTLD_LAZY);
    bool valid = false;
    if (lib != NULL) {
        //			printf("OK\n\tTesting for Necessary functions... ");
        //Testing for Necessary functions
        valid =  (dlsym(lib, "getInstance") != NULL);

        dlclose(lib);
    }else{
        logger.error("checkModule") << "Module doesn't exist! path:" << path;
    }
    //TODO: not sure if dlclose needed if lib == null
    return valid;
}

Module* Loader::load( const module_entry& entry) {
    // for information on dlopen, dlsym, dlerror and dlclose
    // see here: http://linux.die.net/man/3/dlclose

    // open dynamic library (*.so file)
    void *lib = dlopen(getModulePath(entry.localPathToModule,entry.name).c_str(),RTLD_NOW);

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
}
}
