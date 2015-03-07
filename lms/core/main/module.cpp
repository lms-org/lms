#include <core/module.h>
#include <string>
#include <core/lms_exports.h>

namespace lms{
    bool Module::initializeBase(DataManager* d, Loader::module_entry &loaderEntry, logging::Logger *rootLogger) {
        dm = d;
        Module::loaderEntry = loaderEntry;

        // delete uninitialized child logger
        logger.~ChildLogger();
        // C++11 placement new
        new (&logger) logging::ChildLogger(loaderEntry.name, rootLogger);

        return true;
    }

    lms_EXPORT std::string Module::getName() const{
        return loaderEntry.name;
    }

    lms_EXPORT int Module::getPriority() const{
        return priority;
    }
}
