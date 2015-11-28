#include "lms/module_wrapper.h"
#include "lms/runtime.h"

namespace lms {

int ModuleWrapper::getChannelPriority(const std::string &name) const {
    std::map<std::string, int>::const_iterator it = channelPriorities.find(name);

    if(it != channelPriorities.end()) {
        return it->second;
    } else {
        return 0;
    }
}

std::string ModuleWrapper::getChannelMapping(const std::string &mapFrom) const {
    auto it = channelMapping.find(mapFrom);

    if(it != channelMapping.end()) {
        return it->second;
    } else {
        return mapFrom;
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

std::shared_ptr<ServiceWrapper> ModuleWrapper::getServiceWrapper(std::string const& name) {
    return this->runtime->getServiceWrapper(name);
}

}  // namespace lms
