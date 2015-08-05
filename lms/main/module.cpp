#include <string>

#include "lms/module_wrapper.h"
#include <lms/module.h>
#include <lms/lms_exports.h>
#include <lms/datamanager.h>

namespace lms{
    bool Module::initializeBase(DataManager* datamanager, Messaging *messaging,
        std::shared_ptr<ModuleWrapper> wrapper, logging::Logger *rootLogger,
        logging::LogLevel minLogLevel) {

        m_datamanager = datamanager;
        m_messaging = messaging;
        m_wrapper = wrapper;

        // delete uninitialized child logger
        logger.~ChildLogger();
        // C++11 placement new
        new (&logger) logging::ChildLogger(wrapper->name, rootLogger,
            std::unique_ptr<logging::LoggingFilter>(new logging::PrefixAndLevelFilter(minLogLevel)));

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

    lms_EXPORT const type::ModuleConfig* Module::getConfig(const std::string &name){
        return datamanager()->readChannel<type::ModuleConfig>(this, "CONFIG_" + getName() + "_" + name);
    }
    lms_EXPORT bool Module::hasConfig(const std::string &name){
        return datamanager()->hasChannel(this, "CONFIG_" + getName() + "_" + name);
    }
}
