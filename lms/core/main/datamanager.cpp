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

template<typename T>
const T* DataManager::readChannel(const Module *module, const std::string &name) {
    DataChannel &channel = channels[name];

    if(channel.dataWrapper == nullptr) {
        std::cerr << "Channel " << name << " has not yet any writers!" << std::endl;

        channel.dataWrapper = new PointerWrapperImpl<T>();
        channel.dataSize = sizeof(T);
    } else if(channel.dataSize != sizeof(T)) {
        std::cerr << "Channel " << name << " cannot be accessed with wrong type!" << std::endl;
        // TODO do some error handling here
    }

    channel.readers.push_back(module->getName());

    return (const T*)channel.dataWrapper->get();
}

template<typename T>
T* DataManager::writeChannel(const Module *module, const std::string &name) {
    DataChannel &channel = channels[name];

    if(channel.exclusiveWrite) {
        std::cerr << "Channel " << name << " is exclusive write!" << std::endl;
        // TODO do some error handling
    }

    // if dataPointer is null, then the channel did not exist yet
    if(channel.dataWrapper == nullptr) {
        channel.dataWrapper = new PointerWrapperImpl<T>();
        channel.dataSize = sizeof(T);
    } else if(channel.dataSize != sizeof(T)) {
        std::cerr << "Channel " << name << " cannot be accessed with wrong type!" << std::endl;
        // TODO do some error handling here
    }
    /*
     * Fails: F:/UserData/Documents/programmieren/c++/LMS/lms/core/include/core/datamanager.h:95: undefined reference to `lms::Module::getName() const'
     *
     */
    channel.writers.push_back(module->getName());

    return (T*)channel.dataWrapper->get();
}

template<typename T>
T* DataManager::exclusiveWriteChannel(const Module *module, const std::string &name) {
    DataChannel &channel = channels[name];

    if(channel.exclusiveWrite) {
        std::cerr << "Channel " << name << " is exclusive write!" << std::endl;
        // TODO do some error handling
    }

    if(channel.dataWrapper == nullptr) {
        channel.dataWrapper = new PointerWrapperImpl<T>();
        channel.dataSize = sizeof(T);
        channel.exclusiveWrite = true;
    } else if(channel.dataSize != sizeof(T)) {
        std::cerr << "Channel " << name << " cannot be accessed with wrong type!" << std::endl;
        // TODO do some error handling here
    } else if(! channel.writers.empty()) {
        std::cerr << "Channel " << name << " has already writers!" << std::endl;
        // TODO do some error handling here
    }

    channel.writers.push_back(module->getName());

    return (T*)channel.dataWrapper->get();
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
