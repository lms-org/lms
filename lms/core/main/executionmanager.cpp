#include <core/executionmanager.h>
#include <core/datamanager.h>
#include <core/module.h>
#include <core/loader.h>
#include <queue>
#include <iostream>
#include <core/datamanager.h>
#include <stdio.h>
#include <memory>

namespace lms{

ExecutionManager::ExecutionManager(Logger &rootLogger)
    : rootLogger(rootLogger), logger("EXECMGR", &rootLogger), maxThreads(1), valid(false),
    loader(rootLogger), dataManager(rootLogger) {
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
    logger.info() << "load available Modules";
    available  = loader.getModules();
}

void ExecutionManager::enableModule(const std::string &name){
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
            module->initializeBase(&dataManager,it, &rootLogger);
            module->initialize();
            enabledModules.push_back(module);
            invalidate();
        }
    }
}

/**Disable module with the given name, remove it from the cycle-queue */
void ExecutionManager::disableModule(const std::string &name){
    for(size_t i = 0; i< enabledModules.size(); ++i){
        if(enabledModules[i]->getName() == name){
            enabledModules.erase(enabledModules.begin() + 1);
            invalidate();
            return;
        }
    }
    //TODO print error that modules didn't run
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
    // getChannels() returns const& -> you must use a const& here as well
    for(const std::pair<std::string, DataManager::DataChannel> &pair : dataManager.getChannels()){
        // Module* here is ok, you will make a copy of the pointer
        for(Module* reader : pair.second.readers){
            // usual & here
            for(std::vector<Module*> &list: cycleList){
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
    // const& here again
    for(const std::pair<std::string, DataManager::DataChannel>& pair : dataManager.getChannels()){
        for(Module* writer1 : pair.second.writers){
            for(Module* writer2 : pair.second.writers){
                if(writer1->getName() != writer2->getName()){
                    // usual & here
                    for(std::vector<Module*> &list: cycleList){
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

