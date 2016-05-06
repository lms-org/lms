#include "lms/datamanager.h"

namespace lms {

DataManager::DataManager()
        : logger("lms.DataManager") { }

const DataManager::ChannelMap &DataManager::getChannels() const {
    return channels;
}

void DataManager::reset() {
    channels.clear();
}

}  // namespace lms
