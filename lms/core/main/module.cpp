#include <lms/module.h>
#include <string>
#include <lms/lms_exports.h>
#include <lms/datamanager.h>

namespace lms{
    bool Module::initializeBase(DataManager* datamanager, Messaging *messaging,
                                Loader::module_entry &loaderEntry, logging::Logger *rootLogger,
                                logging::LogLevel minLogLevel) {
        m_datamanager = datamanager;
        m_messaging = messaging;
        this->loaderEntry = loaderEntry;

        // delete uninitialized child logger
        logger.~ChildLogger();
        // C++11 placement new
        new (&logger) logging::ChildLogger(loaderEntry.name, rootLogger,
            std::unique_ptr<logging::LoggingFilter>(new logging::PrefixAndLevelFilter(minLogLevel)));

        return true;
    }

    lms_EXPORT std::string Module::getName() const{
        return loaderEntry.name;
    }

    lms_EXPORT int Module::getPriority() const{
        return priority;
    }

    lms_EXPORT std::string Module::getChannelMapping(std::string mapFrom){
        if(loaderEntry.channelMapping.count(mapFrom) == 1){
            return loaderEntry.channelMapping[mapFrom];
        }
        return mapFrom;
    }

    lms_EXPORT const type::ModuleConfig* Module::getConfig(){
        return datamanager()->readChannel<type::ModuleConfig>(this, "CONFIG_" + getName());
    }
}
