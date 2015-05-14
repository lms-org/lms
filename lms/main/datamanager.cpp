#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include <lms/module.h>
#include <lms/datamanager.h>
#include <lms/executionmanager.h>

namespace lms{

DataManager::DataManager(logging::Logger &rootLogger, ExecutionManager &execMgr)
    : logger("DATAMGR", &rootLogger), execMgr(execMgr) {}

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

void DataManager::getWriteAccess(Module *module, const std::string &reqName) {
    std::string name = module->getChannelMapping(reqName);
    DataChannel &channel = channels[name];

    if(channel.exclusiveWrite) {
        logger.error() << "Module " << module->getName() << " requested channel " << name << std::endl
            << " with write access, but the channel is already exclusive.";
    }

    if(checkIfReaderOrWriter(channel, module)) {
        logger.error("getWriteAccess") << "Module " << module->getName() <<
                                    " is already reader or writer of channel "
                                    << name;
    } else {
        execMgr.invalidate();
        channel.writers.push_back(module);
    }
}

void DataManager::getExclusiveWriteAccess(Module *module, const std::string &reqName) {
    std::string name = module->getChannelMapping(reqName);
    DataChannel &channel = channels[name];

    if(channel.exclusiveWrite) {
        logger.error() << "Module " << module->getName() << " requested channel " << name << std::endl
            << " with exclusive write access, but the channel is already exclusive.";
    }

    if(checkIfReaderOrWriter(channel, module)) {
        logger.error("getExclusiveWriteAccess") << "Module " << module->getName() <<
                                    " is already reader or writer of channel "
                                    << name;
    } else {
        execMgr.invalidate();
        channel.exclusiveWrite = true;
        channel.writers.push_back(module);
    }
}

void DataManager::getReadAccess(Module *module, const std::string &reqName) {
    std::string name = module->getChannelMapping(reqName);
    DataChannel &channel = channels[name];

    if(checkIfReaderOrWriter(channel, module)) {
        logger.error("getReadAccess") << "Module " << module->getName() <<
                                    " is already reader or writer of channel "
                                    << name;
    } else {
        execMgr.invalidate();
        channel.readers.push_back(module);
    }
}

bool DataManager::serializeChannel(Module *module, const std::string &reqName, std::ostream &os) {
    std::string name = module->getChannelMapping(reqName);
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

bool DataManager::deserializeChannel(Module *module, const std::string &reqName, std::istream &is) {
    std::string name = module->getChannelMapping(reqName);
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

bool DataManager::hasChannel(Module *module, const std::string &name) const {
    return hasChannel(module->getChannelMapping(name));
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

void DataManager::printMapping()  {
    for(auto const &ch : channels) {
        std::string channelLine = ch.first;
        if(ch.second.exclusiveWrite) {
            channelLine += " (EXCLUSIVE)";
        }
        channelLine = channelLine + " (" + std::to_string(ch.second.dataSize)
                + " Bytes, " + ch.second.dataTypeName + ") :";
        logger.debug("mapping") << channelLine;

        if(! ch.second.readers.empty()) {
            std::string readerLine = "    reading: ";
            for(const Module *reader : ch.second.readers) {
                readerLine += reader->getName() + " ";
            }
            logger.debug("mapping") << readerLine;
        }

        if(! ch.second.writers.empty()) {
            std::string writerLine = "    writing: ";
            for(const Module *writer : ch.second.writers) {
                writerLine += writer->getName() + " ";
            }
            logger.debug("mapping") << writerLine;
        }
    }
}

bool DataManager::checkIfReaderOrWriter(const DataChannel &channel, Module *module) {
    for(Module *mod : channel.readers) {
        if(mod->getName() == module->getName()) {
            return true;
        }
    }

    for(Module *mod : channel.writers) {
        if(mod->getName() == module->getName()) {
            return true;
        }
    }

    return false;
}

}
