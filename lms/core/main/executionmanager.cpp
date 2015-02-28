#include <core/executionmanager.h>
#include <core/datamanager.h>
#include <core/module.h>
#include <core/loader.h>
#include <queue>
#include <core/configurationmanager.h>
#include <iostream>
#include <core/datamanager.h>
#include <stdio.h>

namespace lms{

ExecutionManager::ExecutionManager(DataManager * dataManager){
    ExecutionManager::dataManager = dataManager;
    loader = new Loader();
    valid = false;
}

ExecutionManager::~ExecutionManager () {
    //TODO
    //delete available;
    delete loader;
}

void ExecutionManager::loop() {
    //validate the ExecutionManager
    validate();
    //TODO execute modules like a boss

    //HACK just for testing atm
    for(auto* it: enabledModules){
        it->cycle();
    }

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
            module->initializeBase(dataManager,it);
            module->initialize();
            enabledModules.push_back(module);
            invalidate();
        }
    }
}

/**Disable module with the given name, remove it from the cycle-queue */
void ExecutionManager::disableModule(std::string name){

}
/**
 * @brief ExecutionManager::invalidate calling that method will cause the executionmanager to run validate() in the next loop
 */
void ExecutionManager::invalidate(){
    valid = false;
}

void ExecutionManager::validate(){
    if(!valid){
        valid = true;
        //TODO sort modules
        //TODO validate the DataManager
    }
}

}

