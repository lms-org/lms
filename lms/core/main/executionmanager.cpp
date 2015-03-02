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

ExecutionManager::ExecutionManager() : valid(false) {
}

ExecutionManager::~ExecutionManager () {
    //TODO
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
    available  = loader.getModules();
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
            Module* module = loader.load(it);
            module->initializeBase(&dataManager,it);
            module->initialize();
            enabledModules.push_back(module);
            invalidate();
        }
    }
}

/**Disable module with the given name, remove it from the cycle-queue */
void ExecutionManager::disableModule(std::string name){
    invalidate();
    //TODO
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
        sort();
    }
}

void ExecutionManager::sort(){
    cycleList.clear();
    //add modules to the list
    for(Module* it : enabledModules){
        std::vector<Module*> tmp;
        tmp.push_back(it);
        cycleList.push_back(tmp);
    }
    sortByDataChannel();
    sortByPriority();
}

void ExecutionManager::sortByDataChannel(){
    for(std::vector<Module*> list: cycleList){
        for(Module* master : list){

        }
    }
    //TODO
}

void ExecutionManager::sortByPriority(){
    //TODO
}

}

