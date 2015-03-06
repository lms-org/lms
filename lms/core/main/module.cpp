#include <core/module.h>
#include <string>
#include <core/lms_exports.h>

namespace lms{
    bool Module::initializeBase(DataManager* d, Loader::module_entry &loaderEntry, logging::Logger *rootLogger) {
        dm = d;
        Module::loaderEntry = loaderEntry;
        logger.name = loaderEntry.name;
        logger.parent = rootLogger;
        return true;
    }

    lms_EXPORT std::string Module::getName() const{
        return loaderEntry.name;
    }

    lms_EXPORT int Module::getPriority() const{
        return priority;
    }
}
