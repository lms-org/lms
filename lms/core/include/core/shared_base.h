#ifndef SHARED_SHARED_BASE_H
#define SHARED_SHARED_BASE_H

class DataManager;

#include <core/shared_interface.h>
#include <string>
#include <vector>

namespace lms{

class Shared_Base {
public:
    Shared_Base() { }
    virtual ~Shared_Base() { }
	
    std::string getName(){ return name;}

	void base_initialize(DataManager*);
	virtual bool initialize() = 0;	
	virtual bool deinitialize() = 0;
	virtual bool cycle() = 0;
    //TODO: reset method

protected:
    DataManager* datamanager() { return dm; }
    /**Will be set on loading the module */
    std::string name;
private:
	DataManager* dm;
};
}

#endif
