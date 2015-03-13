#include <lms/executionmanager.h>
#include <lms/datamanager.h>
#include <lms/module.h>
#include <lms/loader.h>
#include <queue>
#include <iostream>
#include <lms/datamanager.h>
#include <cstdio>
#include <memory>
#include <algorithm>

namespace lms{

ExecutionManager::ExecutionManager(logging::Logger &rootLogger)
    : rootLogger(rootLogger), logger("EXECMGR", &rootLogger), maxThreads(1), valid(false),
    loader(rootLogger), dataManager(rootLogger) {
}

ExecutionManager::~ExecutionManager () {
    //TODO
}

void ExecutionManager::loop() {
    //validate the ExecutionManager
    validate();
    //copy cycleList so it can be modified
    cycleListType cycleListTmp = cycleList;

    if(maxThreads == 1){
        //simple single list
        while(cycleListTmp.size() > 0){
            //Iter over all module-vectors and check if they can be executed
            for(size_t i = 0; i < cycleListTmp.size();i++){
                std::vector<Module*>& moduleV = cycleListTmp[i];
                if(moduleV.size() == 1){
                    moduleV[0]->cycle();
                    //remove module from others
                    for(std::vector<Module*>& moduleV2:cycleListTmp){
                        moduleV2.erase(std::remove(moduleV2.begin(),moduleV2.end(),moduleV[0]),moduleV2.end());
                    }
                    //remove moduleV from cycleList
                    cycleListTmp.erase(std::remove(cycleListTmp.begin(),cycleListTmp.end(),moduleV),cycleListTmp.end());
                    i--;

                }
            }
        }
    }else{
        //TODO Woker threads
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
            logger.error("enableModule") << "Module " << name << " is already enabled.";
            return;
        }
    }
    for(auto& it:available){
        if(it.name == name){
            Module* module = loader.load(it);
            module->initializeBase(&dataManager,it, &rootLogger);
            module->initialize();
            enabledModules.push_backdisableModule(module);
            invalidate();
            return;
        }
    }
    logger.error("enable Module") <<"Module " << name << "doesn't exist!";
}

/**Disable module with the given name, remove it from the cycle-queue */
void ExecutionManager::disableModule(const std::string &name){
    for(size_t i = 0; i< enabledModules.size(); ++i){
        if(enabledModules[i]->getName() == name){
            enabledModules.erase(enabledModules.begin() + i);
            invalidate();
            return;
        }
    }

    logger.error("disableModule") << "Module " << name << " was not enabled.";
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
    logger.debug("sort modules") << "sort it size: " << enabledModules.size();
    //TODO sort enabledModules by priority
    //add modules to the list
    for(Module* it : enabledModules){
        std::vector<Module*> tmp;
        tmp.push_back(it);
        cycleList.push_back(tmp);
        std::cout<< "added module!!!!" <<std::endl;
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

