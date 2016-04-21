#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_set>

#include <lms/module.h>
#include <lms/internal/datamanager.h>
#include <lms/internal/executionmanager.h>
#include <lms/internal/dot_exporter.h>

namespace lms {
namespace internal {

DataManager::DataManager()
        : logger("lms.DataManager") { }

const DataManager::ChannelMap &DataManager::getChannels() const {
    return channels;
}

void DataManager::reset() {
    channels.clear();
}

}  // namespace internal
}  // namespace lms
