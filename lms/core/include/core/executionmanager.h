#ifndef SHARED_EXECUTIONMANAGER_H
#define SHARED_EXECUTIONMANAGER_H

#include "shared_base.h"
#include <string>
#include <deque>
#include <map>
#include <sys/time.h>
#include <core/loader.h>

namespace lms{
class DataManager;
class Loader;
#define MEASURE_TIME 1


class ExecutionManager {
    //Für was das?
    ExecutionManager& operator = (const ExecutionManager &) = delete;
public:

    ExecutionManager();
    virtual ~ExecutionManager();

    //Brauchen wir nicht
    void init(DataManager *mgr);

    //auch weg
	void loop();

    //für was das?
	void getTiming(std::map<std::string, double> &t);
    /**Searches the programm for all availabe modules and adds them to the availabe list */
    void loadAvailabelModules();
    /**Enable module with the given name, add it to the cycle-queue */
    void enableModule(std::string name);
    /**Disable module with the given name, remove it from the cycle-queue */
    void disableModule(std::string name);


    void invalidate();
    /**
     * @brief validate if invalidate was called before, it will check if all selected modules are initialised
     * Sorts the modules in the cycle-list
    */
    void validate();
private:

    bool valid;

	Loader *loader;
	DataManager* data;

	struct entry {
		Shared_Base* module;
		std::string name;
        Loader::module_entry entry;
        double last_exec_time;
		struct timeval last_exec_start;
    };
    //cycle list
	std::deque<entry> queue;

    //list of all avaible modules
    std::deque<entry> avaible;

    //brauchen wir eigentlich nicht
	std::map<std::string, std::deque<entry>::iterator> module_mapping;

	void initialize_all();
	std::string getConfigName(entry &e);

//	std::string new_mode;
//    std::string compare_mode;

};
}
#endif
