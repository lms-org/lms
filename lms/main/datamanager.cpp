#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_set>

#include <lms/module.h>
#include <lms/datamanager.h>
#include <lms/executionmanager.h>
#include <lms/extra/dot_exporter.h>

namespace lms {

DataManager::DataManager(Runtime &runtime, ExecutionManager &execMgr)
    : m_runtime(runtime), logger("lms.DataManager"), execMgr(execMgr) {}

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
        channel.writers.push_back(module->wrapper());
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
        channel.writers.push_back(module->wrapper());
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
        channel.readers.push_back(module->wrapper());
    }
}

bool DataManager::serializeChannel(Module *module, const std::string &reqName, std::ostream &os) {
    std::string name = module->getChannelMapping(reqName);
    DataChannel &channel = channels[name];

    if(std::find(channel.readers.begin(), channel.readers.end(), module->wrapper()) == channel.readers.end()
            && std::find(channel.writers.begin(), channel.writers.end(), module->wrapper()) == channel.writers.end()) {
        logger.error("serializeChannel") << "Module " << module->getName()
                                         << " tried to serialize channel " << name
                                         << " without any permissions.";
        return false;
    }

    // if we would use dynamic_cast here, we could remove the serializable
    // flag of data channels, but that is not necessarily faster or better

    if(channel.dataWrapper != nullptr && channel.serializable) {
        const Serializable *data = static_cast<Serializable*>(channel.dataWrapper->get());
        data->lmsSerialize(os);
        return true;
    } else {
        return false;
    }
}

bool DataManager::deserializeChannel(Module *module, const std::string &reqName, std::istream &is) {
    std::string name = module->getChannelMapping(reqName);
    DataChannel &channel = channels[name];

    if(std::find(channel.writers.begin(), channel.writers.end(), module->wrapper()) == channel.writers.end()) {
        logger.error("deserializeChannel") << "Module " << module->getName()
                                         << " tried to deserialize channel " << name
                                         << " without write permissions.";
        return false;
    }

    if(channel.dataWrapper != nullptr && channel.serializable) {
        Serializable *data = static_cast<Serializable*>(channel.dataWrapper->get());
        data->lmsDeserialize(is);
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

void DataManager::releaseChannelsOf(std::shared_ptr<ModuleWrapper> module) {
    for(auto &ch : channels) {
        ch.second.readers.erase(std::remove(ch.second.readers.begin(),
            ch.second.readers.end(), module), ch.second.readers.end());

        ch.second.writers.erase(std::remove(ch.second.writers.begin(),
            ch.second.writers.end(), module), ch.second.writers.end());

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
            for(std::shared_ptr<ModuleWrapper> reader : ch.second.readers) {
                readerLine += reader->name + " ";
            }
            logger.debug("mapping") << readerLine;
        }

        if(! ch.second.writers.empty()) {
            std::string writerLine = "    writing: ";
            for(std::shared_ptr<ModuleWrapper> writer : ch.second.writers) {
                writerLine += writer->name + " ";
            }
            logger.debug("mapping") << writerLine;
        }
    }
}

void DataManager::writeDAG(lms::extra::DotExporter &dot, const std::string &prefix) {
    using lms::extra::DotExporter;

    std::string CONFIG("CONFIG");

    std::unordered_set<std::string> modules;

    for(const auto &ch : channels) {
        if(ch.first.compare(0, CONFIG.size(), CONFIG) == 0) {
            continue;
        }

        dot.shape(DotExporter::Shape::BOX);
        dot.label(ch.first + "\\n" + ch.second.dataTypeName);
        dot.node(prefix + "_" + ch.first);
        dot.reset();
        for(auto writer : ch.second.writers) {
            int prio = writer->getChannelPriority(ch.first);
            if(prio != 0) {
                dot.label(std::to_string(prio));
            }
            dot.edge(prefix + "_" + writer->name, prefix + "_" + ch.first);
            dot.reset();

            modules.insert(writer->name);
        }
        for(auto reader : ch.second.readers) {
            int prio = reader->getChannelPriority(ch.first);
            if(prio != 0) {
                dot.label(std::to_string(prio));
            }
            dot.edge(prefix + "_" + ch.first, prefix + "_" + reader->name);
            dot.reset();

            modules.insert(reader->name);
        }
    }

    for(const auto& mod : modules) {
        dot.label(mod);
        dot.node(prefix + "_" + mod);
        dot.reset();
    }
}

void DataManager::invalidateExecutionManager() {
    execMgr.invalidate();
}

}  // namespace lms
