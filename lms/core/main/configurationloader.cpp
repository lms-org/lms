#include <core/configurationloader.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <core/framework.h>
#include <fstream>
#include <core/type/module_config.h>
#include <core/datamanager.h>

namespace lms{
ConfigurationLoader::ConfigurationLoader(DataManager dataManager) {
    ConfigurationLoader::dataManager = dataManager;
    //Add default values
    addSuffix("");
    addPath("lms/configs/");
}

ConfigurationLoader::~ConfigurationLoader() {
 //TODO
}
void ConfigurationLoader::addPath(std::string path){
    searchDirectories.push_back(path);
}

void ConfigurationLoader::addSuffix(std::string path){
    suffixes.push_back(path);

}

void ConfigurationLoader::validate(){
    //TODO check if files have changed
}


void ConfigurationLoader::getConfig(std::string name){
    //TODO check if Config already exists

    //load config
    std::string configFilePath = getConfigFilePath(name);
    type::ModuleConfig* conf = new type::ModuleConfig();
    conf->loadFromFile(configFilePath);
    //TODO generate Configfile
}

std::string ConfigurationLoader::getConfigFilePath(std::string name){
    std::ifstream ifs;
    for(std::string& dir : searchDirectories){
        for(std::vector<std::string>::reverse_iterator suffix = suffixes.rbegin(); suffix != suffixes.rend(); ++suffix) {
            if((*suffix).size() == 0){
                ifs.open (Framework::programDirectory()+dir+name +".lconf", std::ifstream::in);
                if(ifs.is_open()){
                    ifs.close();
                    return dir+name+"_"+".lconf";
                }
            }else{
                ifs.open (Framework::programDirectory()+dir+name+"_"+(*suffix)+".lconf", std::ifstream::in);
                if(ifs.is_open()){
                    ifs.close();
                    return dir+name+"_"+(*suffix)+".lconf";
                }
            }
        }
    }
    return NULL;

}

}
