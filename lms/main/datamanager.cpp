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
        : logger("lms.DataManager"), execMgr(execMgr), m_runtime(runtime) { }

const DataManager::ChannelMap &DataManager::getChannels() const {
    return channels;
}

void DataManager::releaseChannelsOf(std::shared_ptr<ModuleWrapper> module) {
    std::vector<std::string> channelsToRemove;

    for(auto &ch : channels) {
        ch.second->readers.erase(std::remove(ch.second->readers.begin(),
            ch.second->readers.end(), module), ch.second->readers.end());

        ch.second->writers.erase(std::remove(ch.second->writers.begin(),
            ch.second->writers.end(), module), ch.second->writers.end());

        if(ch.second->writers.empty() && ch.second->readers.empty()) {
            channelsToRemove.push_back(ch.first);
        }
    }

    for(auto const& channelName : channelsToRemove) {
        channels.erase(channelName);
    }

    invalidateExecutionManager();
}

void DataManager::printMapping() {
    for (auto const &ch : channels) {
        std::string channelLine = ch.first;
        channelLine = channelLine + "(" + ch.second->main->typeName() + ") :";
        logger.debug("mapping") << channelLine;

        if (!ch.second->readers.empty()) {
            std::string readerLine = "    reading: ";
            for (std::shared_ptr<ModuleWrapper> reader : ch.second->readers) {
                readerLine += reader->name() + " ";
            }
            logger.debug("mapping") << readerLine;
        }

        if (!ch.second->writers.empty()) {
            std::string writerLine = "    writing: ";
            for (std::shared_ptr<ModuleWrapper> writer : ch.second->writers) {
                writerLine += writer->name() + " ";
            }
            logger.debug("mapping") << writerLine;
        }
    }
}

void DataManager::writeDAG(lms::extra::DotExporter &dot, const std::string &prefix) {
    using lms::extra::DotExporter;

    std::unordered_set<std::string> modules;

    for (const auto &ch : channels) {
        dot.shape(DotExporter::Shape::BOX);
        dot.label(ch.first + "\\n" + ch.second->main->typeName());
        dot.node(prefix + "_" + ch.first);
        dot.reset();
        for (auto writer : ch.second->writers) {
            int prio = writer->getChannelPriority(ch.first);
            if (prio != 0) {
                dot.label(std::to_string(prio));
            }
            dot.edge(prefix + "_" + writer->name(), prefix + "_" + ch.first);
            dot.reset();

            modules.insert(writer->name());
        }
        for (auto reader : ch.second->readers) {
            int prio = reader->getChannelPriority(ch.first);
            if (prio != 0) {
                dot.label(std::to_string(prio));
            }
            dot.edge(prefix + "_" + ch.first, prefix + "_" + reader->name());
            dot.reset();

            modules.insert(reader->name());
        }
    }

    for (const auto &mod : modules) {
        dot.label(mod);
        dot.node(prefix + "_" + mod);
        dot.reset();
    }
}

void DataManager::invalidateExecutionManager() {
    execMgr.invalidate();
}

void DataManager::reset() {
    channels.clear();
}

}  // namespace lms
