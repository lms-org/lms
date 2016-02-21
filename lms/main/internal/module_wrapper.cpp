#include "lms/internal/module_wrapper.h"
#include "lms/internal/runtime.h"

namespace lms {
namespace internal {

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
    return this->runtime()->getServiceWrapper(name);
}

bool ModuleWrapper::enabled() const {
    return m_enabled;
}

Module* ModuleWrapper::instance() const {
    return m_moduleInstance.get();
}

void ModuleWrapper::load(void* instance) {
    m_enabled = instance != nullptr;
    m_moduleInstance.reset(static_cast<Module*>(instance));
}

void ModuleWrapper::unload() {
    m_enabled = false;
    m_moduleInstance.reset();
}

Runtime* ModuleWrapper::runtime() const {
    return m_runtime;
}

void ModuleWrapper::runtime(Runtime* runtime) {
    m_runtime = runtime;
}

std::string ModuleWrapper::interfaceFunction() const {
    return "lms_module_" + clazz();
}

}  // namespace internal
}  // namespace lms
