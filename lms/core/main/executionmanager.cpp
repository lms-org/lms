#include <core/executionmanager.h>
#include <core/datamanager.h>
#include <core/shared_base.h>
#include <core/loader.h>
#include <queue>
#include <core/configurationmanager.h>
#include <iostream>


ExecutionManager::ExecutionManager() : data(NULL) {
	loader = new Loader();
}

void ExecutionManager::init(DataManager *d) {
	data = d;
}

ExecutionManager::~ExecutionManager () {
	int i = 0;
	for (auto it = queue.rbegin(); it != queue.rend(); ++it, i++) {
		data->setActiveModule(it->module);
		printf("Deinit: (%i/%i) %s \n", i, (int)queue.size(), it->module->getName().c_str());
		it->module->deinitialize();
	}
    for (auto it = avaible.rbegin(); it != avaible.rend(); ++it) {
		loader->unload(it->module);
	}
    delete loader;
}

void ExecutionManager::loop() {
    //struct timeval tv;
    struct timeval tv_exe_start;
    struct timeval tv_exe_end;
    //struct timeval tv_module_start;
    //struct timeval tv_module_end;

    gettimeofday(&tv_exe_start, NULL);

/*	for (auto it = queue.begin(); it != queue.end(); ++it) {
		gettimeofday(&tv_module_start, NULL);
    }*/

	//Has a mode change occurred?
    //if(new_mode != "")
//    if (new_mode != compare_mode) {
//		applyNewMode();
//	}
	gettimeofday(&tv_exe_end, NULL);
    double delta = (tv_exe_end.tv_sec - tv_exe_start.tv_sec) * 1e3 + (tv_exe_end.tv_usec - tv_exe_start.tv_usec) * 1e-3;
    //printf("\033[031m Execution time: %f ms \n \033[0m", delta);
    //printf("\033[2J\033[1;1H");
}

void ExecutionManager::loadAvailabelModules(){

}

/**Enable module with the given name, add it to the cycle-queue */
void ExecutionManager::enableModule(std::string name){

}

/**Disable module with the given name, remove it from the cycle-queue */
void ExecutionManager::disableModule(std::string name){

}

std::string ExecutionManager::getConfigName(ExecutionManager::entry &e) {
	return e.entry.place + "_" + e.entry.name + "_selected";
}

void ExecutionManager::getTiming(std::map<std::string, double> &t) {
	for(auto it = queue.begin(); it != queue.end(); ++it) {
		t[it->name] = it->last_exec_time;
	}
}


