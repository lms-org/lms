#include <core/configurationmanager.h>
#include <unistd.h>
#include <limits.h>

ConfigurationManager::ConfigurationManager() {
	char path[PATH_MAX] = {0};
	if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
		perror("readlink failed");
		exit(1);
	}

	configuration_path=path;
	configuration_path=configuration_path.substr(0, configuration_path.rfind("/"));
	configuration_path=configuration_path.substr(0, configuration_path.rfind("/"));
	configuration_path=configuration_path.substr(0, configuration_path.rfind("/"));
	configuration_path += "/config/";
	printf("Configuration Directory: %s\n", configuration_path.c_str());
}

ConfigurationManager::~ConfigurationManager() {
	for (auto it = storage.begin(); it != storage.end(); ++it) {
		if (it->second->flush()) {
			printf("Flushing config file %s...\n", it->first.c_str());
		} else printf ("No changes in Config %s\n", it->first.c_str());
		delete it->second;
	}
	storage.clear();
}

void ConfigurationManager::flush() {
	for (auto it = storage.begin(); it != storage.end(); ++it) {
		if(it->second->flush()) {
			printf("Flushing config file %s...\n", it->first.c_str());
		}
	}
}

ConfigFile* ConfigurationManager::getConfig(const char *cname) {
	std::string name = cname;
	auto it = storage.find(name);
	if (it == storage.end()) {
		ConfigFile *cfg = NULL;
		if (!subpath.empty()) {
			for (auto it2 = subpath.begin(); it2 != subpath.end(); ++it2) {
				printf("Trying %s", (configuration_path + *it2 + name + ".conf").c_str());
				cfg = new ConfigFile(configuration_path + *it2 + name + ".conf");
				if (cfg->good())
					break;
				delete cfg;
				cfg = NULL;
			}
		}
		if (cfg == NULL) {
			std::string config_path = configuration_path + name + ".conf";
			printf("Opening %s\n", config_path.c_str());
			cfg = new ConfigFile(config_path);
		}

		storage.insert(std::pair<std::string, ConfigFile*>(name, cfg));
		return cfg;
	} else
		return it->second;
}

void ConfigurationManager::update() {
	auto backup = storage;
	storage.clear();
	for (auto it = backup.begin(); it != backup.end(); it++) {
		it->second->flush();
		delete it->second;
		getConfig(it->first.c_str());
	}
}
