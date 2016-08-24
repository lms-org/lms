#include <string>

#include <lms/module.h>
#include <lms/lms_exports.h>
#include "lms/internal/framework.h"

namespace lms {
bool Module::init() { return initialize(); }

void Module::destroy() { deinitialize(); }

bool Module::initialize() { return true; }

bool Module::deinitialize() { return true; }

void Module::configsChanged() {}

bool Module::initBase(const internal::ModuleInfo &info, internal::Framework *fw) {
    m_info = info;
    m_fw = fw;
    m_datamanager = &(fw->dataManager());

    logger.name = info.name;
    logger.threshold = info.log;

    return true;
}

lms_EXPORT std::string Module::getName() const { return m_info.name; }

lms_EXPORT bool Module::isMainThread() const {
    return m_info.mainThread;
}

lms_EXPORT const Config &Module::config(const std::string &name) {
    return m_info.configs[name];
}

lms_EXPORT bool Module::hasConfig(const std::string &name) {
    return m_info.configs.find(name) != m_info.configs.end();
}

int Module::cycleCounter() { return m_fw->executionManager().cycleCounter(); }

bool Module::isEnableSave() const {
    return m_fw->isEnableSave();
}

bool Module::isEnableLoad() const {
    return m_fw->isEnableLoad();
}

std::string Module::saveLogFile(std::string const &name) {
    return m_fw->saveLogObject(name, false);
}

std::string Module::loadLogFile(std::string const &name) {
    return m_fw->loadLogObject(name, false);
}

std::string Module::saveLogDir(std::string const &name) {
    return m_fw->saveLogObject(name, true);
}

std::string Module::loadLogDir(std::string const &name) {
    return m_fw->loadLogObject(name, true);
}

std::shared_ptr<Service>
Module::_getService(std::string const &name) {
    return m_fw->getService(name);
}

std::string Module::mapChannel(const std::string &channelName) {
    return m_info.mapChannel(channelName).first;
}

void Module::gainReadAccess(const std::string &channelName) {
    auto mapped = m_info.mapChannel(channelName);
    m_fw->executionManager().getModuleChannelGraph().readChannel(mapped.first, this,
                                                            mapped.second);
    m_fw->executionManager().invalidate();
}

void Module::gainWriteAccess(const std::string &channelName) {
    auto mapped = m_info.mapChannel(channelName);
    m_fw->executionManager().getModuleChannelGraph().writeChannel(mapped.first, this,
                                                             mapped.second);
    m_fw->executionManager().invalidate();
}

Messaging* Module::messaging() const {
    LMS_EXCEPTION("Messaging is currently not implemented"); // TODO
}

}
