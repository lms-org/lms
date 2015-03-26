#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include <lms/module.h>
#include <lms/datamanager.h>

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
        it->second.dataWrapper = nullptr;
    }
}

void DataManager::getWriteAccess(Module *module, const std::string &name) {
    DataChannel &channel = channels[name];

    if(channel.exclusiveWrite) {
        logger.error() << "Module " << module->getName() << " requested channel " << name << std::endl
            << " with write access, but the channel is already exclusive.";
    }

    channel.writers.push_back(module);
}

void DataManager::getExclusiveWriteAccess(Module *module, const std::string &name) {
    DataChannel &channel = channels[name];

    if(channel.exclusiveWrite) {
        logger.error() << "Module " << module->getName() << " requested channel " << name << std::endl
            << " with exclusive write access, but the channel is already exclusive.";
    }

    channel.exclusiveWrite = true;
    channel.writers.push_back(module);
}

void DataManager::getReadAccess(Module *module, const std::string &name) {
    DataChannel &channel = channels[name];

    channel.readers.push_back(module);
}

bool DataManager::serializeChannel(Module *module, const std::string &name, std::ostream &os) {
    DataChannel &channel = channels[name];

    if(std::find(channel.readers.begin(), channel.readers.end(), module) == channel.readers.end()
            && std::find(channel.writers.begin(), channel.writers.end(), module) == channel.writers.end()) {
        logger.error("serializeChannel") << "Module " << module->getName()
                                         << " tried to serialize channel " << name
                                         << " without any permissions.";
        return false;
    }

    // if we would use dynamic_cast here, we could remove the serializable
    // flag of data channels, but that is not necessarily faster or better

    if(channel.dataWrapper != nullptr && channel.serializable) {
        const Serializable *data = static_cast<Serializable*>(channel.dataWrapper->get());
        data->serialize(os);
        return true;
    } else {
        return false;
    }
}

bool DataManager::deserializeChannel(Module *module, const std::string &name, std::istream &is) {
    DataChannel &channel = channels[name];

    if(std::find(channel.writers.begin(), channel.writers.end(), module) == channel.writers.end()) {
        logger.error("deserializeChannel") << "Module " << module->getName()
                                         << " tried to deserialize channel " << name
                                         << " without write permissions.";
        return false;
    }

    if(channel.dataWrapper != nullptr && channel.serializable) {
        Serializable *data = static_cast<Serializable*>(channel.dataWrapper->get());
        data->deserialize(is);
        return true;
    } else {
        return false;
    }
}

const std::map<std::string,DataManager::DataChannel>& DataManager::getChannels() const {
    return channels;
}

bool DataManager::hasChannel(const std::string &name) const {
    return channels.count(name) == 1;
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
            for(const Module *reader : ch.second.readers) {
                std::cout << reader->getName() << " ";
            }
            std::cout << std::endl;
        }

        if(! ch.second.writers.empty()) {
            std::cout << "\twriting: ";
            for(const Module *writer : ch.second.writers) {
                std::cout << writer->getName() << " ";
            }
            std::cout << std::endl;
        }
    }
}

const type::ModuleConfig* DataManager::getConfig(Module *module, const std::string &name, const std::vector<std::string> privateDirectories) {
    if(! hasChannel(name)) {
        setChannel<type::ModuleConfig>(name, configLoader.loadConfig(name,privateDirectories));
    }

    return readChannel<type::ModuleConfig>(module, name);
}

}
