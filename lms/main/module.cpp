#include <string>

#include "lms/module_wrapper.h"
#include <lms/module.h>
#include <lms/lms_exports.h>
#include <lms/datamanager.h>
#include "lms/module_config.h"
#include "lms/executionmanager.h"

namespace lms{
    bool Module::initializeBase(DataManager* datamanager, Messaging *messaging,
                                ExecutionManager *execManager,
        std::shared_ptr<ModuleWrapper> wrapper,
        logging::Level minLogLevel) {

        m_datamanager = datamanager;
        m_messaging = messaging;
        m_executionManager = execManager;
        m_wrapper = wrapper;

        logger.name = wrapper->name;
        logger.threshold = minLogLevel;

        return true;
    }

    lms_EXPORT std::string Module::getName() const{
        return m_wrapper->name;
    }

    lms_EXPORT int Module::getPriority() const{
        return m_wrapper->writePriority;
    }

    lms_EXPORT extra::PrecisionTime Module::getExpectedRuntime() const {
        return m_wrapper->expectedRuntime;
    }

    lms_EXPORT ModuleWrapper::ExecutionType Module::getExecutionType() const {
        return m_wrapper->executionType;
    }

    lms_EXPORT std::string Module::getChannelMapping(const std::string &mapFrom) {
        auto it = m_wrapper->channelMapping.find(mapFrom);

        if(it != m_wrapper->channelMapping.end()) {
            return it->second;
        } else {
            return mapFrom;
        }
    }

    lms_EXPORT const ModuleConfig* Module::getConfig(const std::string &name){
        return datamanager()->readChannel<ModuleConfig>(this, "CONFIG_" + getName() + "_" + name);
    }
    lms_EXPORT bool Module::hasConfig(const std::string &name){
        return datamanager()->hasChannel(this, "CONFIG_" + getName() + "_" + name);
    }

    int Module::cycleCounter() {
        return m_executionManager->cycleCounter();
    }
}
