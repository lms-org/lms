#include <iostream>
#include <stdio.h>
#include <limits.h>
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

namespace lms{

Loader::Loader(Logger &rootLogger) : logger("LOADER", &rootLogger) {
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
        }
        ifs.close();
    }else{
        //found some folder with no config-file
    }
}

void Loader::unload(Module* a) {
    delete a;
}


std::string Loader::getModulePath(const std::string &modulePath, const std::string &moduleName) {
    return pathToModules+modulePath+"lib"+moduleName+".so";
}

}

