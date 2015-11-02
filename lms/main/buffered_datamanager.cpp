#include "lms/buffered_datamanager.h"

namespace lms {

std::string BufferedDataManager::channelType(std::string const& channelName) const {
    auto it = m_dataChannels.find(channelName);

    if(it == m_dataChannels.end()) {
        return "";
    } else {
        return it->second.dataTypeName;
    }
}

}  // namespace lms
