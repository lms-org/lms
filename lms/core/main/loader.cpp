#include <iostream>
#include <stdio.h>
#include <limits.h>
#include <dirent.h>
#include <cstring>
#include <dlfcn.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <core/loader.h>
#include <core/module.h>
#include <pugixml.hpp>
#include <core/logger.h>

namespace lms{
template<typename _Target>
union converter {
    void* src;
    _Target target;
};

Loader::Loader() {
    char path[PATH_MAX];
    memset (path, 0, PATH_MAX);
    if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
        perror("readlink failed");
        exit(1);
    }
    //get programmdirectory
    programm_directory = path;
    programm_directory = programm_directory.substr(0, programm_directory.rfind("/"));
    programm_directory = programm_directory.substr(0, programm_directory.rfind("/"));

    //printf("Programm Directory: %s\n", programm_directory.c_str());

    Loader::pathToModules = programm_directory + "/" +"external/modules/";

}

Loader::moduleList Loader::getModules() {
    static std::string loadConfigName= "loadConfig.xml";
    std::string configFilePath;
    moduleList list;

    DIR *dp = NULL;
    dirent *d = NULL;
    //TODO get list of all module-folders
    if((dp = opendir( pathToModules.c_str())) == NULL) {
        printf("Could not Open Directory: %s: ", pathToModules.c_str());
        perror("Reason: ");
        return Loader::moduleList();
    }

    while((d = readdir(dp)) != NULL) {
        //Check if file is folder
        //TODO wont work on solar (maybe), . and .. folder is found
        if (d->d_type == DT_DIR) {
            //get path
            configFilePath = pathToModules+d->d_name + "/"+loadConfigName;
            //  std::cout <<"path: " + configFilePath + modulePath"\n"<<std::endl;

            std::ifstream ifs;

            ifs.open (configFilePath, std::ifstream::in);

            if(ifs.is_open()){
                //config-file exists
                pugi::xml_document doc;
                pugi::xml_parse_result result = doc.load(ifs);

                if (result){

                    //parse modules
                    pugi::xml_node modulesNode =doc.child("modules");

                    for (pugi::xml_node_iterator it = modulesNode.begin(); it != modulesNode.end(); ++it){
                        //parse module content
                        std::string moduleName = it->child("name").child_value();
                        //set mainFolder (folder after modules
                        std::string localPathToModule = d->d_name;
                        localPathToModule = localPathToModule +"/";
                        std::string pathTmp = it->child("path").child_value();
                        /*
                            //remove spaces, caused by xml parsing
                            moduleName.erase (std::remove (moduleName.begin(), moduleName.end(), ' '), moduleName.end());
                            localPathToModule.erase (std::remove (localPathToModule.begin(), localPathToModule.end(), ' '), localPathToModule.end());
                            */
                        /**Add module-subdirectory.
                             * Needed if the module contains more than one submodule that can run
                            */
                        localPathToModule = localPathToModule+pathTmp;

                        std::string modulePath = getModulePath(localPathToModule,moduleName);
                        std::cout << "modulePath: " + modulePath << std::endl;
                        //check if module is valid
                        if(checkModule(modulePath.c_str())){
                            std::cout << "Module valid " + moduleName << "add it to list" << std::endl;
                        }else{
                            std::cout << "Module invalid! " + moduleName <<std::endl;
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
                }
                //close loadConfig-file
                ifs.close();
            }else{
                //found some folder with no config-file
            }

        }
    }
    return list;
}

bool Loader::checkModule(const char* path){
    void* lib = dlopen (path, RTLD_LAZY);
    bool valid = false;
    if (lib != NULL) {
        //			printf("OK\n\tTesting for Necessary functions... ");
        //Testing for Necessary functions
        valid =  (dlsym(lib, "getName") != NULL && dlsym(lib, "getInstance") != NULL);

        dlclose(lib);
    }else{
        std::cout << "Module doesn't exist! path:" <<path << std::endl;
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

void Loader::unload(Module* a) {
    delete a;
}


std::string Loader::getModulePath(std::string modulePath, std::string moduleName) {
    return pathToModules+modulePath+"lib"+moduleName+".so";
}

}

