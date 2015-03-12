#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include <unistd.h>

#include <lms/type/module_config.h>
#include <lms/logger.h>

/**
 * TODO rename file
 *
 * TODO shared und private Configs unterscheiden
 */
namespace lms{
/**
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
    type::ModuleConfig loadConfig(const std::string &name);
    /**
     * @brief addPath
     * @param path will be added to the searchDirectories list
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
     * @brief searchDirectories relative path: LMS/<searchDirectories>
     */
    std::vector<std::string> searchDirectories;

    std::vector<std::string> suffixes;
    /**
     * @brief getConfigFilePath
     * @param name
     * @return the configfilePath given by the suffixes and directories
     */
    std::string getConfigFilePath(const std::string &name);
};
}
#endif
