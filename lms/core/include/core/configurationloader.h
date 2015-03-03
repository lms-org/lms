#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include <unistd.h>
/**
 *TODO rename file
 */
namespace lms{
class ConfigurationLoader {
public:
    ConfigurationLoader();
    virtual ~ConfigurationLoader();
    void getConfig(std::string name);
    void addPath(std::string path);
    void addSuffix(std::string path);
    void validate();

private:
    /**
     * @brief searchDirectories relative path: LMS/<searchDirectories>
     */
    std::vector<std::string> searchDirectories;

    std::vector<std::string> suffixes;

    std::string getConfigFilePath(std::string name);
};
}
#endif
