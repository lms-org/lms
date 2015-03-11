#include <core/configurationloader.h>
#include <unistd.h>
#include <climits>
#include <cstdio>
#include <core/framework.h>
#include <fstream>
#include <core/type/module_config.h>
#include <cstdlib>

namespace lms{
class DataManager;

ConfigurationLoader::ConfigurationLoader(logging::Logger &rootLogger)
    : logger("CONFIGLOADER", &rootLogger) {
    //Add default values
    addSuffix("");

    // get a configuration directory from environment variables
    char *envLmsConfigPath = getenv("LMS_CONFIG_PATH");
    if(envLmsConfigPath != NULL) {
        addPath(envLmsConfigPath);
    }

    // from the build directory
    addPath("configs/");
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


type::ModuleConfig ConfigurationLoader::loadConfig(const std::string &name){
    //load config
    std::string configFilePath(getConfigFilePath(name));
    if(configFilePath.empty()) {
        logger.warn("loadConfig") << "config file path is empty";
    }

    logger.info("loadConfig") << configFilePath;

    type::ModuleConfig conf;
    if(! conf.loadFromFile(configFilePath)) {
        logger.error("loadConfig") << "could not load config file " << configFilePath;
    }
    return conf;
}

std::string ConfigurationLoader::getConfigFilePath(const std::string &name) {
    std::ifstream ifs;
    for(std::string& dir : searchDirectories){
        for(std::vector<std::string>::reverse_iterator suffix = suffixes.rbegin(); suffix != suffixes.rend(); ++suffix) {

            logger.info("getConfigFilePath") << Framework::programDirectory() << " " << dir << " "
                << *suffix << " " << name;

            if((*suffix).size() == 0){
                ifs.open (Framework::programDirectory()+dir+name +".lconf", std::ifstream::in);
                if(ifs.is_open()){
                    ifs.close();
                    return Framework::programDirectory()+dir+name+".lconf";
                }
            }else{
                ifs.open (Framework::programDirectory()+dir+name+"_"+(*suffix)+".lconf", std::ifstream::in);
                if(ifs.is_open()){
                    ifs.close();
                    return Framework::programDirectory()+dir+name+"_"+(*suffix)+".lconf";
                }
            }
        }
    }
    return "";

}

}
