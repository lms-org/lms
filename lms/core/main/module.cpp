#include <core/module.h>
#include <string>
namespace lms{
    bool Module::initializeBase(DataManager* d,Loader::module_entry &loaderEntry) {
        dm = d;
        Module::loaderEntry = loaderEntry;
        return true;
    }

    std::string Module::getName(){
        return loaderEntry.name;
    }
}
