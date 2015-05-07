#include <cstdio>
#include <queue>
#include <iostream>
#include <memory>
#include <algorithm>

#include "lms/executionmanager.h"
#include "lms/datamanager.h"
#include "lms/module.h"
#include "lms/loader.h"
#include "lms/datamanager.h"

namespace lms {

ExecutionManager::ExecutionManager(logging::Logger &rootLogger)
    : rootLogger(rootLogger), logger("EXECMGR", &rootLogger), maxThreads(1),
      valid(false), loader(rootLogger), dataManager(rootLogger, *this),
      messaging() {
}

ExecutionManager::~ExecutionManager () {
    for(std::vector<Module*>::iterator it = enabledModules.begin();
        it != enabledModules.end(); ++it) {

        if(! (*it)->deinitialize()) {
            logger.error("disableModule")
                    << "Deinitialize failed for module " << (*it)->getName();
        }

        loader.unload(*it);
    }

    enabledModules.clear();
}

DataManager& ExecutionManager::getDataManager() {
    return dataManager;
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

    // TODO load or unload modules or do anything else
    for(std::string message : messaging.receive("mod-unload")) {
        // TODO do something
    }

    // Remove all messages from the message queue
    messaging.resetQueue();
}

void ExecutionManager::addAvailableModule(const Loader::module_entry &mod){
    logger.info() << "Add module " << mod.name;
    available.push_back(mod);
}

void ExecutionManager::enableModule(const std::string &name, lms::logging::LogLevel minLogLevel){
    //Check if module is already enabled
    for(auto* it:enabledModules){
        if(it->getName() == name){
            logger.error("enableModule") << "Module " << name << " is already enabled.";
            return;
        }
    }
    for(auto& it:available){
        if(it.name == name){
            logger.debug("enable Module") <<"enabling Module: " <<name;
            Module* module = loader.load(it);
            module->initializeBase(&dataManager, &messaging, it, &rootLogger, minLogLevel);

            if(module->initialize()){
                enabledModules.push_back(module);
            }else{
                logger.error("enable Module") <<"Enabling Module "<< name << " failed";
            }
            invalidate();
            return;
        }
    }
    logger.error("enable Module") <<"Module " << name << "doesn't exist!";
}

/**Disable module with the given name, remove it from the cycle-queue */
bool ExecutionManager::disableModule(const std::string &name) {
    for(std::vector<Module*>::iterator it = enabledModules.begin();
        it != enabledModules.end(); ++it) {

        if((*it)->getName() == name) {
            if(! (*it)->deinitialize()) {
                logger.error("disableModule")
                        << "Deinitialize failed for module " << name;
            }
            loader.unload(*it);
            enabledModules.erase(it);
            invalidate();
            return true;
        }
    }

    logger.error("disableModule") << "Tried to disable module " << name
                                  << ", but was not enabled.";
    return false;
}

void ExecutionManager::invalidate(){
    valid = false;
}

void ExecutionManager::validate(){
    if(!valid){
        valid = true;
        //TODO dataManager.validate()
        sort();
    }
}

void ExecutionManager::setMaxThreads(int maxThreads) {
    this->maxThreads = maxThreads;
}

void ExecutionManager::printCycleList() {
    for(const std::vector<Module*> &list : cycleList) {
        std::string line;

        for(Module* mod : list) {
            line += mod->getName() + " ";
        }

        logger.debug("cycleList") << line;
    }
}

void ExecutionManager::sort(){
    cycleList.clear();
    logger.debug("sort modules") << "sort it size: " << enabledModules.size();
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
    // getChannels() returns const& -> you must use a const& here as well
    for(const std::pair<std::string, DataManager::DataChannel> &pair : dataManager.getChannels()){
        // Module* here is ok, you will make a copy of the pointer
        for(Module* reader : pair.second.readers){
            // usual & here
            for(std::vector<Module*> &list: cycleList){
                Module *toAdd = list[0];
                //add all writers to the reader
                if(toAdd->getName() == reader->getName()){
                    for(Module* writer : pair.second.writers){
                        list.push_back(writer);
                        logger.info("adding: ") <<toAdd->getName() << " <- "<< writer->getName();

                    }
                }
            }
        }
    }
}

void ExecutionManager::sortByPriority(){
    // const& here again
    for(const std::pair<std::string, DataManager::DataChannel>& pair : dataManager.getChannels()){
        //sort writer-order read-order isn't needed as readers don't change the dataChannel
        //check insert is in the writers-list BEGIN
        for(Module* writer1 : pair.second.writers){
            for(std::vector<Module*> &list: cycleList){
                Module* insert = list[0];
                //check insert is in the writers-list END
                if(insert->getName() == writer1->getName()){
                    //yes it is contained
                    for(Module* writer2 : pair.second.writers){
                        //add all writers(2) with a higher priority to the list
                        if(writer2->getPriority() > insert->getPriority()){
                            list.push_back(writer2);
                        }
                    }
                }
            }
        }
    }
}

}  // namespace lms
