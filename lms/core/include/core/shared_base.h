#ifndef SHARED_SHARED_BASE_H
#define SHARED_SHARED_BASE_H

class DataManager;

//für was das?
#define GET_CHANNEL(handle,target) \
	target=(__decltype(target))datamanager()->get_channel(handle);

#include <core/shared_interface.h>
#include <string>
#include <vector>

namespace lms{
//was soll das?
#define IMPLEMENT_BASE \
	public: \
	std::string getName() { return ::getName(); }

class Shared_Base {
public:
    Shared_Base() { }
    virtual ~Shared_Base() { }
	
	virtual std::string getName() = 0;

	void base_initialize(DataManager*);
	virtual bool initialize() = 0;	
	virtual bool deinitialize() = 0;
	virtual bool cycle() = 0;
    //TODO: reset method

protected:
    DataManager* datamanager() { return dm; }
private:
	DataManager* dm;
};
}

#endif
