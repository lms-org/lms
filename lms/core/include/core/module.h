#ifndef LMS_MODULE_H
#define LMS_MODULE_H

#include <string>
#include <vector>
#include <memory>

#include <core/loader.h>

namespace lms{

class DataManager;
class Module {
public:
    Module():priority(0) { }
    virtual ~Module() { }
	
    std::string getName() const;
    /**
     * called by the framework itself at module-creation
    */
    bool initializeBase(DataManager* d,Loader::module_entry& loaderEntry, Logger *rootLogger);

    int getPriority() const;

    /**
     * TODO
     * @brief initialize
     * @param d
     * @return
     */
    virtual bool initialize() = 0;
	virtual bool deinitialize() = 0;
	virtual bool cycle() = 0;

    //TODO: reset method
    // The following implementation is fully backwards compatible:
    // Modules can override reset if they can implement it
    virtual void reset() {}
    // If a module overrides reset() then isResettable() should be overriden to return true
    virtual bool isResettable() { return false; }

protected:
    DataManager* datamanager() { return dm; }
    ChildLogger logger;
private:
    Loader::module_entry loaderEntry;
	DataManager* dm;
    int priority;
};
}

#endif /*LMS_MODULE_H*/
