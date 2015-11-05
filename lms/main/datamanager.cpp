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

    // TODO delete everything
}

void DataManager::getWriteAccess(Module *module, const std::string &reqName) {
    writeChannel<Void>(module, reqName);
}

void DataManager::getReadAccess(Module *module, const std::string &reqName) {
    readChannel<Void>(module, reqName);
}

bool DataManager::serializeChannel(Module *module, const std::string &reqName, std::ostream &os) {
    return readChannel<Void>(module, reqName).serialize(os);
}

bool DataManager::deserializeChannel(Module *module, const std::string &reqName, std::istream &is) {
    return writeChannel<Void>(module, reqName).deserialize(is);
}

const DataManager::ChannelMap &DataManager::getChannels() const {
    return channels;
}

bool DataManager::hasChannel(const std::string &name) const {
    return channels.count(name) == 1;
}

bool DataManager::hasChannel(Module *module, const std::string &name) const {
    return hasChannel(module->getChannelMapping(name));
}

void DataManager::releaseChannelsOf(std::shared_ptr<ModuleWrapper> module) {
    // TODO fix me :(
    // TODO call invalidate

    /*for(auto &ch : channels) {
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
    }*/
}

void DataManager::printMapping()  {
    for(auto const &ch : channels) {
        std::string channelLine = ch.first;
        channelLine = channelLine + "(" + ch.second->main->typeName() + ") :";
        logger.debug("mapping") << channelLine;

        if(! ch.second->readers.empty()) {
            std::string readerLine = "    reading: ";
            for(std::shared_ptr<ModuleWrapper> reader : ch.second->readers) {
                readerLine += reader->name + " ";
            }
            logger.debug("mapping") << readerLine;
        }

        if(! ch.second->writers.empty()) {
            std::string writerLine = "    writing: ";
            for(std::shared_ptr<ModuleWrapper> writer : ch.second->writers) {
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
        dot.label(ch.first + "\\n" + ch.second->main->typeName());
        dot.node(prefix + "_" + ch.first);
        dot.reset();
        for(auto writer : ch.second->writers) {
            int prio = writer->getChannelPriority(ch.first);
            if(prio != 0) {
                dot.label(std::to_string(prio));
            }
            dot.edge(prefix + "_" + writer->name, prefix + "_" + ch.first);
            dot.reset();

            modules.insert(writer->name);
        }
        for(auto reader : ch.second->readers) {
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
