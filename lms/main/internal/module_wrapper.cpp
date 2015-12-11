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

std::string ModuleWrapper::libname() const {
    return m_libname;
}

void ModuleWrapper::libname(std::string const& libname) {
    m_libname = libname;
}

std::string ModuleWrapper::name() const {
    return m_name;
}

void ModuleWrapper::name(std::string const& name) {
    m_name = name;
}

bool ModuleWrapper::enabled() const {
    return m_enabled;
}

Module* ModuleWrapper::instance() const {
    return m_moduleInstance.get();
}

void ModuleWrapper::instance(Module* instance) {
    m_enabled = instance != nullptr;
    m_moduleInstance.reset(instance);
}

Runtime* ModuleWrapper::runtime() const {
    return m_runtime;
}

void ModuleWrapper::runtime(Runtime* runtime) {
    m_runtime = runtime;
}

}  // namespace internal
}  // namespace lms
