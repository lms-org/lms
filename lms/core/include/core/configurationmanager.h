#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <core/configfile.h>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>

class ConfigurationManager {
public:
	ConfigurationManager();
	virtual ~ConfigurationManager();
	
	void flush();

	ConfigFile *getConfig(const char* name);

	std::vector<std::string> search_subdirs() { return subpath; }
    const std::string path() { return configuration_path; }
	void update();
private:
	std::map<std::string, ConfigFile*> storage;

	std::string configuration_path;
	std::vector<std::string> subpath;
};
#endif
