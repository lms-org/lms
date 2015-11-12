#include "lms/module_wrapper.h"

namespace lms {

int ModuleWrapper::getChannelPriority(const std::string &name) const {
    std::map<std::string, int>::const_iterator it = channelPriorities.find(name);

    if(it != channelPriorities.end()) {
        return it->second;
    } else {
        return 0;
    }
}

void ModuleWrapper::update(ModuleWrapper && other) {
    this->channelPriorities = other.channelPriorities;
    this->channelMapping = other.channelMapping;

    // preserve location of existing ModuleConfigs
    for(auto&& entry : other.configs) {
        this->configs[entry.first] = entry.second;
    }
}

}  // namespace lms
