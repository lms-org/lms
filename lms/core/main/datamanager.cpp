#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <core/handle.h>
#include <core/module.h>
#include <core/datamanager.h>

namespace lms{

DataManager::~DataManager() {
    // TODO destruct all dataPointers

    // zum löschen:
    // destruktor kann nicht direkt aufgerufen werden, da typ nicht bekannt.
    // deshalb:
    // - doch handle
    // - oder: releaseChannel in deinitialize aufrufen

    for(auto it = channels.begin(); it != channels.end(); it++) {
        delete it->second.dataWrapper;
    }
}

const std::map<std::string,DataManager::DataChannel>& DataManager::getChannels() const {
    return channels;
}

/*void DataManager::print_mapping() {
	for (auto it = management.begin(); it != management.end(); ++it) {
		printf("\n");
		if (it->second.registered)
			printf(COLOR_YELLOW);
		else
			printf(COLOR_RED);

        // TODO lösche COLOR_WHITE
		printf ("%12s" COLOR_WHITE ": " GREEN("%15s") " [%6i] --> ",
			it->first.c_str(),
			it->second.registerer.c_str(),
            it->second.length
			);
		for (auto iit = it->second.acquired.begin();
				iit != it->second.acquired.end(); ++iit) {
			if (*iit != it->second.registerer)
				printf("%-15s ", iit->c_str());
		}

	}
}*/
}
