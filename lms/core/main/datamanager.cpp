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

void DataManager::releaseChannelsOf(const Module *module) {
    for(auto &ch : channels) {
        for(auto it = ch.second.readers.begin(); it != ch.second.readers.end(); ++it) {
            if(*it == module) {
                ch.second.readers.erase(it);
                break;
            }
        }

        for(auto it = ch.second.writers.begin(); it != ch.second.writers.end(); ++it) {
            if(*it == module) {
                ch.second.writers.erase(it);
                break;
            }
        }

        if(ch.second.writers.empty()) {
            ch.second.exclusiveWrite = false;

            if(ch.second.readers.empty()) {
                // TODO delete the channel
            }
        }
    }
}

void DataManager::printMapping() const {
    for(auto const &ch : channels) {
        std::cout << ch.first;
        if(ch.second.exclusiveWrite) {
            std::cout << " (EXCLUSIVE)";
        }
        std::cout << " (" <<ch.second.dataSize << " Bytes) :" << std::endl;

        if(! ch.second.readers.empty()) {
            std::cout << "\treading: ";
            for(Module *reader : ch.second.readers) {
                std::cout << reader->getName() << " ";
            }
            std::cout << std::endl;
        }

        if(! ch.second.writers.empty()) {
            std::cout << "\twriting: ";
            for(Module *writer : ch.second.writers) {
                std::cout << writer->getName() << " ";
            }
            std::cout << std::endl;
        }
    }
}

}
