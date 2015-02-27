#include <core/executionmanager.h>
#include <core/datamanager.h>
#include <core/module.h>
#include <core/loader.h>
#include <queue>
#include <core/configurationmanager.h>
#include <iostream>
#include <core/datamanager.h>

namespace lms{

ExecutionManager::ExecutionManager(DataManager * dataManager){
    ExecutionManager::dataManager = dataManager;
    loader = new Loader();
}

ExecutionManager::~ExecutionManager () {
    //TODO
    //delete available;
    delete loader;
}

void ExecutionManager::loop() {
    //validate the ExecutionManager
    validate();
    //TODO execute modules
}

void ExecutionManager::loadAvailabelModules(){
    printf("load AvailabelModules");
    available  = loader->getModules();
}

void ExecutionManager::enableModule(std::string name){
    //Check if module is already enabled
    for(auto* it:enabledModules){
        if(it->getName() == name){
            //TODO throw error
            return;
        }
    }
    for(auto& it:available){
        if(it.name == name){
            Module* module = loader->load(it);

        }
    }
}

/**Disable module with the given name, remove it from the cycle-queue */
void ExecutionManager::disableModule(std::string name){

}

void ExecutionManager::validate(){
    //TODO sort modules
}

}

