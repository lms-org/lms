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

    //TODO
    //validate();
    //validateDataChannels();
}

void ExecutionManager::sortByDataChannel(){
    for(std::pair<std::string, DataManager::DataChannel> pair : dataManager.getChannels()){
        for(Module* reader : pair.second.readers){
            for(std::vector<Module*> list: cycleList){
                for(Module* toAdd : list){
                    //add all writers to the needed modules
                    for(Module* writer : pair.second.writers){
                        if(toAdd->getName() == reader->getName()){
                            list.push_back(writer);
                        }
                    }
                }
            }
        }
    }
}

void ExecutionManager::sortByPriority(){
    for(std::pair<std::string, DataManager::DataChannel> pair : dataManager.getChannels()){
        for(Module* writer1 : pair.second.writers){
            for(Module* writer2 : pair.second.writers){
                if(writer1->getName() != writer2->getName()){
                    for(std::vector<Module*> list: cycleList){
                        for(Module* insert : list){
                            if(writer1->getPriority() >= writer2->getPriority()){
                                if(insert->getName() == writer2->getName()){
                                    list.push_back(writer1);
                                }
                            }else{
                                if(insert->getName() == writer1->getName()){
                                    list.push_back(writer2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

}

