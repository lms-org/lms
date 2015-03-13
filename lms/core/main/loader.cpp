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

#include <lms/loader.h>
#include <lms/module.h>
#include <pugixml.hpp>
#include <lms/logger.h>
#include <lms/framework.h>

namespace lms{

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
                //module entry, used for loading
                struct module_entry entry;
                //parse module content
                std::string moduleName = it->child("name").child_value();
                //set mainFolder (folder after modules
                std::string localPathToModule = moduleFolderName;
                localPathToModule = localPathToModule +"/";
                std::string pathTmp = it->child("path").child_value();
                localPathToModule = localPathToModule+pathTmp;

                std::string libname;
                if(it->child("libname")){
                    libname = it->child("libname").child_value();
                }else{
                    libname = moduleName;
                }
                std::string modulePath = getModulePath(localPathToModule,libname);
                logger.info() << "modulePath: " + modulePath;
                //check if module is valid
                if(checkModule(modulePath.c_str())){
                    logger.info() << "Module valid " << moduleName << " -> add it to list";
                }else{
                    logger.warn() << "Module invalid! " << moduleName;
                    continue;
                }
                //get string mapping
                for(pugi::xml_node_iterator mappingIt : it->child("mapping")){
                    entry.stringMapping[mappingIt->child_value("from")] = mappingIt->child_value("to");
                }
                //get config file paths
                for(pugi::xml_node_iterator localPath : it->child("configPaths")){
                    //create absolute path
                    entry.configPaths.push_back(pathToModules+moduleFolderName+"/"+localPath->child_value());
                }
                //add module to list
                entry.localPathToModule = localPathToModule;
                entry.name = moduleName;
                entry.libname = libname;
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


std::string Loader::getModulePath(const std::string &modulePath, const std::string &libname) {
    return pathToModules+modulePath+"lib"+libname+".so";
}

}

