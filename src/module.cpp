#include <string>

#include <lms/module.h>
#include <lms/lms_exports.h>
#include "internal/framework.h"

namespace lms {

struct Module::Private {
    lms::internal::Framework *m_fw;
    lms::internal::ModuleInfo m_info;
};

Module::Module() : logger(""), dptr(new Private) {}
Module::~Module() {
    delete dptr;
}

bool Module::init() { return initialize(); }

void Module::destroy() { deinitialize(); }

bool Module::initialize() { return true; }

bool Module::deinitialize() { return true; }

void Module::configsChanged() {}

bool Module::initBase(const internal::ModuleInfo &info, internal::Framework *fw) {
    dfunc()->m_info = info;
    dfunc()->m_fw = fw;

    logger.name = info.name;
    logger.threshold = info.log;

    return true;
}

lms_EXPORT std::string Module::getName() const { return dfunc()->m_info.name; }

lms_EXPORT bool Module::isMainThread() const {
    return dfunc()->m_info.mainThread;
}

lms_EXPORT const Config &Module::config(const std::string &name) {
    return dfunc()->m_info.configs[name];
}

lms_EXPORT bool Module::hasConfig(const std::string &name) {
    return dfunc()->m_info.configs.find(name) != dfunc()->m_info.configs.end();
}

int Module::cycleCounter() { return dfunc()->m_fw->executionManager().cycleCounter(); }

bool Module::isEnableSave() const {
    return dfunc()->m_fw->isEnableSave();
}

bool Module::isEnableLoad() const {
    return dfunc()->m_fw->isEnableLoad();
}

std::string Module::saveLogFile(std::string const &name) {
    return dfunc()->m_fw->saveLogObject(name, false);
}

std::string Module::loadLogFile(std::string const &name) {
    return dfunc()->m_fw->loadLogObject(name, false);
}

std::string Module::saveLogDir(std::string const &name) {
    return dfunc()->m_fw->saveLogObject(name, true);
}

std::string Module::loadLogDir(std::string const &name) {
    return dfunc()->m_fw->loadLogObject(name, true);
}

std::shared_ptr<Service>
Module::_getService(std::string const &name) {
    return dfunc()->m_fw->getService(name);
}

std::string Module::mapChannel(const std::string &channelName) {
    return dfunc()->m_info.mapChannel(channelName).first;
}

void Module::gainReadAccess(const std::string &channelName) {
    auto mapped = dfunc()->m_info.mapChannel(channelName);
    if(dfunc()->m_fw->executionManager().getModuleChannelGraph().readChannel(mapped.first, this,
                                                            mapped.second)) {
        dfunc()->m_fw->executionManager().invalidate();
    }
}

void Module::gainWriteAccess(const std::string &channelName) {
    auto mapped = dfunc()->m_info.mapChannel(channelName);
    if(dfunc()->m_fw->executionManager().getModuleChannelGraph().writeChannel(mapped.first, this,
                                                             mapped.second)) {
        dfunc()->m_fw->executionManager().invalidate();
    }
}

bool Module::isChannelInUse(const std::string &channelName) const {
    auto mapped = dfunc()->m_info.mapChannel(channelName);
    return dfunc()->m_fw->executionManager().getModuleChannelGraph().hasReaders(mapped.first);
}

Messaging* Module::messaging() const {
    LMS_EXCEPTION("Messaging is currently not implemented"); // TODO
}

DataManager &Module::datamanager() {
    return dfunc()->m_fw->dataManager();
}

bool Module::pauseRuntime(std::string const &name) {
    logger.error() << "Not implemented";
    return false;
}

void Module::pauseRuntime() {
    logger.error() << "Not implemented";
}

bool Module::resumeRuntime(std::string const &name, bool reset) {
    logger.error() << "Not implemented";
    return false;
}

void Module::exitRuntime(int status) {
    dfunc()->m_fw->stop();
}

}
