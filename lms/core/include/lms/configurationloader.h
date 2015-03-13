#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include <unistd.h>

#include <lms/type/module_config.h>
#include <lms/logger.h>


namespace lms{
/**
 * TODO add xml support
 * @brief The ConfigurationLoader class used to load config-files for Modules
 */
class ConfigurationLoader {
public:
    ConfigurationLoader(logging::Logger &rootLogger);
    virtual ~ConfigurationLoader();
    /**
     * @brief loadConfig
     * @param name
     * Adds the config-data-channel to the DataManager
     * Will reload the config if it was already loaded
     * //TODO Man könnte auch alle configs nacheinander laden wodruch die werte überschrieben würden
     */
    type::ModuleConfig loadConfig(const std::string &name, const std::vector<std::string> & privateDirectories = {});
    /**
     * @brief addPath
     * @param path, absolute path that will be added to the searchDirectories list
     */
    void addPath(const std::string &path);
    /**
     * last added suffix will be used first for searching configfiles
     * @brief addSuffix
     * @param path
     */
    void addSuffix(const std::string &path);
    /**
     * @brief validate checks if config-files changed and updates the datachannels if needed
     */
    void validate();

private:
    logging::ChildLogger logger;
    /**
     * @brief searchDirectories absolute public paths
     */
    std::vector<std::string> searchDirectories;

    std::vector<std::string> suffixes;
    /**
     * @brief getSuffixConfig prefers private directories over public directories
     * @param name
     * @return the path given by the suffixes and directories or "" if no valid file was found
     */
    std::string getSuffixConfig(const std::string &name,const std::vector<std::string> & privateDirectories);
    /**
     * @brief getDefaultConfigPath prefers private directories over public directories
     * @param name
     * @return the path given by the directories (private and public) or "" if no valid file was found
     */
    std::string getDefaultConfigPath(const std::string &name,const std::vector<std::string> & privateDirectories);
    /**
     * @brief getPath just searches for a file
     * @param name
     * @param suffix
     * @param directories
     * @return the path or "" if no valid file was found
     */
    std::string getPath(const std::string &name,const std::string& suffix, const std::vector<std::string>& directories);
};
}
#endif
