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
        valid =  (dlsym(lib, "getName") != NULL && dlsym(lib, "getInstance") != NULL);

        dlclose(lib);
    }else{
        logger.error("checkModule") << "Module doesn't exist! path:" << path;
    }
    //TODO: not sure if dlclose needed if lib == null
    return valid;
}

Module* Loader::load( const module_entry& entry) {
    void *lib = dlopen(getModulePath(entry.localPathToModule,entry.name).c_str(),RTLD_NOW);
    if(lib == NULL) {
        //TODO write error msg
    }
    void* func = dlsym(lib, "getInstance");
    if (func == NULL) {
        perror("dlsym");
        //TODO write error msg
    }
    ///Union-Hack to avoid a warning message
    converter <void*(*)()> conv;
    conv.src = func;

    return (Module*)conv.target();
}
}
