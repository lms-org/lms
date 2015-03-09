#include <iostream>
#include <cstdio>
#include <climits>
#include <dirent.h>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <core/loader.h>
#include <core/module.h>
#include <pugixml.hpp>
#include <core/logger.h>
#include <core/framework.h>

#ifdef _WIN32
    // TODO
#else
    #include <dlfcn.h>
#endif

namespace lms{
    
template<typename _Target>
union converter {
    void* src;
    _Target target;
};

Loader::Loader(logging::Logger &rootLogger) : logger("LOADER", &rootLogger) {
    pathToModules = Framework::programDirectory() + "external/modules/";
}

Loader::moduleList Loader::getModules() {
    static std::string loadConfigName= "loadConfig.xml";
    std::string configFilePath;
    moduleList list;

    DIR *dp = NULL;
    dirent *d = NULL;
    if((dp = opendir( pathToModules.c_str())) == NULL) {
        logger.error() << "Could not Open Directory: " << pathToModules;
        perror("Reason: ");
        return Loader::moduleList();
    }

    while((d = readdir(dp)) != NULL) {
        /*
         * Hack only works because handleLoadConfig cares about it :D
         * plattformspecific checks or boost::filesystem or some other lib would be usefull but as we don't wanna add so much code for just one need it's fine
         */
        configFilePath = pathToModules+d->d_name + "/"+loadConfigName;
        //add modules to list (if they exist)
        handleLoadConfig(configFilePath,d->d_name,list);
        // }
    }

    closedir(dp);

    return list;
}

void Loader::handleLoadConfig(const std::string &configFilePath,
                              const std::string &moduleFolderName, moduleList& list){
    std::ifstream ifs;
    ifs.open (configFilePath, std::ifstream::in);
    if(ifs.is_open()){
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(ifs);

        if (result){

            //parse modules
            pugi::xml_node modulesNode =doc.child("modules");

            for (pugi::xml_node_iterator it = modulesNode.begin(); it != modulesNode.end(); ++it){
                //parse module content
                std::string moduleName = it->child("name").child_value();
                //set mainFolder (folder after modules
                std::string localPathToModule = moduleFolderName;
                localPathToModule = localPathToModule +"/";
                std::string pathTmp = it->child("path").child_value();
                localPathToModule = localPathToModule+pathTmp;

                std::string modulePath = getModulePath(localPathToModule,moduleName);
                logger.info() << "modulePath: " + modulePath;
                //check if module is valid
                if(checkModule(modulePath.c_str())){
                    logger.info() << "Module valid " << moduleName << " -> add it to list";
                }else{
                    logger.warn() << "Module invalid! " << moduleName;
                    continue;
                }
                //add module to list
                struct module_entry entry;
                entry.localPathToModule = localPathToModule;
                entry.name = moduleName;
                //TODO
                entry.localPathToConfigs = "";
                list.push_back(entry);
            }
        }else{
            //TODO can't parse plugin-xml
            logger.error("handleLoadConfig") << "Can't parse plugin-xml";
        }
        ifs.close();
    }else{
        //found some folder with no config-file
        logger.warn("handleLoadConfig") << "There was no plugin-xml in folder";
    }
}

void Loader::unload(Module* a) {
    delete a;
}


std::string Loader::getModulePath(const std::string &modulePath, const std::string &moduleName) {
    return pathToModules+modulePath+"lib"+moduleName+".so";
}

bool Loader::checkModule(const char* path){
#ifdef _WIN32
    // TODO
    return false;
#else
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
#endif
}

Module* Loader::load( const module_entry& entry) {
#ifdef _WIN32
    // TODO
    return nullptr;
#else
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
#endif
}

}

