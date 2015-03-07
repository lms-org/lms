#include <core/configurationloader.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <core/framework.h>
#include <fstream>
#include <core/type/module_config.h>
#include <core/datamanager.h>

namespace lms{
ConfigurationLoader::ConfigurationLoader(DataManager* dataManager)
    : dataManager(dataManager) {
    //Add default values
    addSuffix("");
    addPath("lms/configs/");
}

ConfigurationLoader::~ConfigurationLoader() {
 //TODO
}
void ConfigurationLoader::addPath(const std::string &path){
    searchDirectories.push_back(path);
}

void ConfigurationLoader::addSuffix(const std::string &path){
    suffixes.push_back(path);

}

void ConfigurationLoader::validate(){
    //TODO check if files have changed
}


void ConfigurationLoader::loadConfig(const std::string &name){
    //load config
    std::string configFilePath(getConfigFilePath(name));
    if(configFilePath.length() == 0) {
        //TODO print error
        return;
    }
    type::ModuleConfig* conf =  dataManager->getChannel<type::ModuleConfig>(name);
    if(conf == NULL) {
        conf = new type::ModuleConfig();
        conf->loadFromFile(configFilePath);
        dataManager->setChannel(name,conf);
    }
}

std::string ConfigurationLoader::getConfigFilePath(const std::string &name) {
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
    return "";

}

}
