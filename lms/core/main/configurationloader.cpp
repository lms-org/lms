#include <lms/configurationloader.h>
#include <unistd.h>
#include <climits>
#include <cstdio>
#include <lms/framework.h>
#include <fstream>
#include <lms/type/module_config.h>
#include <cstdlib>

namespace lms{
class DataManager;

ConfigurationLoader::ConfigurationLoader(logging::Logger &rootLogger)
    : logger("CONFIGLOADER", &rootLogger) {

    // get a configuration directory from environment variables
    char *envLmsConfigPath = getenv("LMS_CONFIG_PATH");
    if(envLmsConfigPath != NULL) {
        addPath(envLmsConfigPath);
    }

    // from the build directory
    addPath(Framework::programDirectory()+"configs/");
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


type::ModuleConfig ConfigurationLoader::loadConfig(const std::string &name, const std::vector<std::string> & privateDirectories){
    //load config
    std::string defaultConfig = getDefaultConfigPath(name,privateDirectories);
    std::string customConfig = getSuffixConfig(name,privateDirectories);
    if(defaultConfig.empty()) {
        logger.warn("loadConfig") << "No default config with name" <<name;
    }

    type::ModuleConfig conf;

    //load default config
    if(!defaultConfig.empty()){
        logger.debug("load default config: ") << defaultConfig.size();
        if(!conf.loadFromFile(defaultConfig)) {
            logger.error("loadConfig") << "could not load config file " << defaultConfig;
        }
    }
    //overload config if possible
    if(!customConfig.empty()){
        logger.debug("load suffix config: ") << defaultConfig << defaultConfig.size();
        if(!conf.loadFromFile(customConfig)) {
            logger.error("loadConfig") << "could not load config file " << customConfig;
        }
    }
    return conf;
}

std::string ConfigurationLoader::getDefaultConfigPath(const std::string &name, const std::vector<std::string> & privateDirectories ){
    std::string path = getPath(name,"",privateDirectories);
    if(path.size() > 0)
        return path;
    path = getPath(name,"",searchDirectories);
    if(path.size() > 0)
        return path;
    return "";
}

std::string ConfigurationLoader::getSuffixConfig(const std::string &name,const std::vector<std::string> & privateDirectories) {
    for(std::vector<std::string>::reverse_iterator suffix = suffixes.rbegin(); suffix != suffixes.rend(); ++suffix) {
        std::string path = getPath(name,*suffix,privateDirectories);
        if(path.size() > 0)
            return path;
    }
    for(std::vector<std::string>::reverse_iterator suffix = suffixes.rbegin(); suffix != suffixes.rend(); ++suffix) {
        std::string path = getPath(name,*suffix,searchDirectories);
        if(path.size() > 0)
            return path;
    }
    return "";
}

std::string ConfigurationLoader::getPath(const std::string &name,const std::string& suffix, const std::vector<std::string>& directories){
    std::ifstream ifs;
    for(const std::string& dir : directories){
        logger.info("getConfigFilePath") << dir << " "
                                         << suffix << " " << name;
        if(suffix.empty() == 0){
            ifs.open (dir+name +".lconf", std::ifstream::in);
            if(ifs.is_open()){
                ifs.close();
                return dir+name+".lconf";
            }
        }else{
            ifs.open (dir+name+"_"+suffix+".lconf", std::ifstream::in);
            if(ifs.is_open()){
                ifs.close();
                return dir+name+"_"+suffix+".lconf";
            }
        }
    }
    return "";
}

}
