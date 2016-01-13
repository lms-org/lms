#include <cstdio>
#include <queue>
#include <iostream>
#include <memory>
#include <algorithm>

#include "lms/internal/executionmanager.h"
#include "lms/internal/datamanager.h"
#include "lms/module.h"
#include "lms/internal/module_wrapper.h"
#include "lms/internal/loader.h"
#include "lms/internal/datamanager.h"
#include "lms/internal/profiler.h"
#include "lms/logger.h"
#include "lms/internal/dot_exporter.h"
#include "lms/internal/runtime.h"
#include "lms/internal/framework.h"

namespace lms {
namespace internal {

ExecutionManager::ExecutionManager(Profiler &profiler, Runtime &runtime)
    : m_runtimeName(runtime.name()),
      logger(runtime.name() + ".ExecutionManager"), m_numThreads(1),
      m_multithreading(false),
      valid(false), dataManager(runtime, *this),
      m_messaging(), m_cycleCounter(-1), running(true),
      m_profiler(profiler), m_runtime(runtime) {
}

ExecutionManager::~ExecutionManager () {
    stopRunning();

    disableAllModules();
}

void ExecutionManager::disableAllModules() {
    for(ModuleList::reverse_iterator it = enabledModules.rbegin();
        it != enabledModules.rend(); ++it) {

        try {
            it->second->instance()->deinitialize();
        } catch(std::exception &e) {
            logger.error("disableModule") << "Module " << it->first << " threw exception: "
                << e.what();
        }
    }

    for(ModuleList::reverse_iterator it = available.rbegin();
        it != available.rend(); ++it) {
        dataManager.releaseChannelsOf(it->second);
        m_runtime.framework().moduleLoader().unload(it->second.get());
    }

    enabledModules.clear();

    invalidate();
}

DataManager& ExecutionManager::getDataManager() {
    return dataManager;
}

void ExecutionManager::loop() {
    // Remove all messages from the message queue
    m_messaging.resetQueue();

    m_cycleCounter ++;

    //validate the ExecutionManager
    validate();

    if(! m_multithreading) {
        for(Module* mod : sortedCycleList) {
            m_dog.beginModule(mod->getName());

            profiler().markBegin(m_runtimeName + "." + mod->getName());

            if(m_runtime.framework().isDebug()) {
                logger.debug("executeBegin") << mod->getName();
            }

            mod->cycle();
            if(m_runtime.framework().isDebug()) {
                logger.debug("executeEnd") << mod->getName();
            }

            profiler().markEnd(m_runtimeName + "." + mod->getName());

            m_dog.endModule();
        }
    }else{
        logger.info() << "Cycle start";

        // if thread pool is not yet initialized then do it now
        if(threadPool.empty()) {
            for(int threadNum = 1; threadNum <= m_numThreads; threadNum++) {
                threadPool.push_back(std::thread([threadNum, this] () {
                    threadFunction(threadNum);
                }));
            }
        }

        {
            std::lock_guard<std::mutex> lck(mutex);
            // copy cycleList so it can be modified
            cycleListTmp = cycleList;
            numModulesToExecute = cycleListTmp.countNodes();

            // inform all threads that there are new jobs to do
            cv.notify_all();
        }

        {
            // wait until the cycle list is empty
            /*std::unique_lock<std::mutex> lck(mutex);
            cv.wait(lck, [this] () {
                return numModulesToExecute == 0;
            });*/

            threadFunction(0);
        }

        logger.info() << "Cycle end";
    }
}

void ExecutionManager::threadFunction(int threadNum) {
    // Thread function

    std::unique_lock<std::mutex> lck(mutex);

    while(running) {
        // wait until something is in the cycleList
        cv.wait(lck, [this, threadNum]() {
            // the main thread stops
            if(threadNum == 0 && numModulesToExecute == 0) {
                return true;
            }

            return hasExecutableModules(threadNum);
        });

        if(numModulesToExecute == 0) {
            break;
        }

        Module* executableModule;

        bool found = cycleListTmp.getFree(executableModule, [threadNum] (Module* mod) {
            ExecutionType execType = mod->getExecutionType();
            return (execType == ExecutionType::ONLY_MAIN_THREAD && threadNum == 0) ||
                (execType == ExecutionType::NEVER_MAIN_THREAD && threadNum != 0);
        });

        if(found) {
            // if an executable module was found
            // then delete it from the cycleListTmp
            cycleListTmp.removeNode(executableModule);

            if(m_runtime.framework().isDebug()) {
                logger.info() << "Thread " << threadNum << " executes "
                              << executableModule->getName();
            }

            // now we can execute it
            lck.unlock();
            profiler().markBegin(m_runtimeName + "." + executableModule->getName());
            executableModule->cycle();
            profiler().markEnd(m_runtimeName + "." + executableModule->getName());
            lck.lock();

            if(m_runtime.framework().isDebug()) {
                logger.info() << "Thread " << threadNum << " executed "
                              << executableModule->getName();
            }

            // now we should delete the executed module from
            // the dependencies of other modules
            cycleListTmp.removeEdgesFrom(executableModule);

            numModulesToExecute --;

            // now inform our fellow threads that something new
            // can be executed
            cv.notify_all();
        }
    }
}

bool ExecutionManager::hasExecutableModules(int thread) {
    if(! running) {
        return true;
    }

    if(cycleListTmp.empty()) {
        return false;
    }

    return cycleListTmp.hasFree([thread] (Module* mod) -> bool {
        ExecutionType execType = mod->getExecutionType();
        return (execType == ExecutionType::ONLY_MAIN_THREAD && thread == 0) ||
                ((execType == ExecutionType::NEVER_MAIN_THREAD && thread != 0));
    });
}

void ExecutionManager::stopRunning() {
    {
        std::lock_guard<std::mutex> lck(mutex);
        running = false;
        cv.notify_all();
    }

    for(std::thread &th : threadPool) {
        th.join();
    }
}

bool ExecutionManager::installModule(std::shared_ptr<ModuleWrapper> mod) {
    std::string moduleName = mod->name();

    if(available.count(moduleName) != 0) {
        logger.error("addAvailableModule") << "Tried to add available "
            << "module " << mod->name() << " but was already available.";
        return false;
    }

    available[moduleName] = mod;
    return true;
}

void ExecutionManager::bufferModule(std::shared_ptr<ModuleWrapper> mod) {
    std::unique_lock<std::mutex> lock(updateMutex);
    update[mod->name()] = mod;
}

void ExecutionManager::updateOrInstall() {
    std::unique_lock<std::mutex> lock(updateMutex);
    for(auto const& mod : update) {
        auto it = available.find(mod.first);

        if(it == available.end()) {
            // module was not installed yet
            available[mod.first] = mod.second;
        } else {
            // update relevant attributes
            it->second->update(std::move(*mod.second.get()));
        }
    }

    if(! update.empty()) {
        fireConfigsChangedEvent();
    }

    // all buffered modules were processed...
    update.clear();
}

bool ExecutionManager::enableModule(const std::string &name, lms::logging::Level minLogLevel){
    //Check if module is already enabled
    if(enabledModules.count(name) != 0) {
        logger.error("enableModule") << "Module " << name << " is already enabled.";
        return false;
    }

    auto it = available.find(name);

    if(it == available.end()) {
        logger.error("enableModule") <<"Module " << name << " doesn't exist!";
        return false;
    }

    std::shared_ptr<ModuleWrapper> mod = it->second;

    if (! m_runtime.framework().moduleLoader().load(mod.get())) {
        return false;
    }

    Module *module = mod->instance();
    module->initializeBase(mod,minLogLevel);

    if (! module->initialize()) {
        logger.error("enableModule") << "Module " << name << " failed to init()";
        return false;
    }

    enabledModules[mod->name()] = mod;
    invalidate();
    return true;
}

bool ExecutionManager::disableModule(const std::string &name) {
    auto it = enabledModules.find(name);

    if (it == enabledModules.end()) {
        logger.error("disableModule") << "Tried to disable module " << name
            << ", but was not enabled.";
        return false;
    }

    try {
        if(! it->second->instance()->deinitialize()) {
            logger.error("disableModule")
            << "Deinitialize failed for module " << name;
        }
    } catch(std::exception &e) {
        logger.error("disableModule") << "Module " << name << " threw exception: "
            << e.what();
        return false;
    }

    m_runtime.framework().moduleLoader().unload(it->second.get());

    enabledModules.erase(it);

    dataManager.releaseChannelsOf(it->second);

    invalidate();
    return true;
}

void ExecutionManager::invalidate(){
    valid = false;
}

void ExecutionManager::validate(){
    if(!valid){
        valid = true;
        sort();

        sortedCycleList.clear();
        bool success = cycleList.topoSort(sortedCycleList);

        if(! success) {
            logger.error("validate") << "Module graph has circle";
        }
    }
}

void ExecutionManager::numThreads(int num) {
    m_numThreads = num;
}

void ExecutionManager::numThreadsAuto() {
    m_numThreads = std::thread::hardware_concurrency();
}

int ExecutionManager::numThreads() const {
    return m_numThreads;
}

void ExecutionManager::enabledMultithreading(bool flag) {
    m_multithreading = flag;
}

bool ExecutionManager::enabledMultithreading() const {
    return m_multithreading;
}

void ExecutionManager::printCycleList(DAG<Module *> &clist) {
    clist.removeTransitiveEdges();

    for(auto const& pair : clist) {
        std::string line(pair.first->getName() + " (");

        for(Module* mod : pair.second) {
            line += " " + mod->getName();
        }

        line += " )";

        logger.debug("cycleList") << line;
    }
}

void ExecutionManager::printCycleList() {
    printCycleList(cycleList);
}

void ExecutionManager::sort(){
    cycleList.clear();
    logger.debug("sort") << "No. of enabled modules: " << enabledModules.size();
    //add modules to the list
    for(auto it : enabledModules){
        cycleList.node(it.second->instance());
    }
    sortModules();
}

void ExecutionManager::sortModules(){
    //find modules that use the same data-channel
    for(const std::pair<std::string,std::shared_ptr<DataChannelInternal>> &pair : dataManager.getChannels()){
        //create one list
        std::vector<std::shared_ptr<ModuleWrapper>> all;
        for(std::shared_ptr<ModuleWrapper> r1 : pair.second->readers){
            all.push_back(r1);
        }
        for(std::shared_ptr<ModuleWrapper> w1 : pair.second->writers){
            all.push_back(w1);
        }

        //don't do size -1!
        for(size_t i = 0; i < all.size(); i++) {
            std::shared_ptr<ModuleWrapper> mw1 = all[i];
            int prio1 = mw1->getChannelPriority(pair.first);

            for(size_t k = i+1; k < all.size(); k++) {
                std::shared_ptr<ModuleWrapper> mw2 = all[k];
                int prio2 = mw2->getChannelPriority(pair.first);

                if(prio1 < prio2) {
                    addModuleDependency(mw1,mw2);
                } else if(prio2 < prio1) {
                    addModuleDependency(mw2,mw1);
                } else if(prio1 == prio2) {
                    //check if it's reader vs writer
                    bool mw1Write = false;
                    bool mw2Write = false;

                    for(std::shared_ptr<ModuleWrapper> r1 : pair.second->writers){
                        if(r1->name() == mw1->name()){
                            mw1Write = true;
                        }else if(r1->name() == mw2->name()){
                            mw2Write = true;
                        }
                    }
                    if(mw1Write && !mw2Write){
                        addModuleDependency(mw2,mw1);
                    }else if(!mw1Write && mw2Write){
                        addModuleDependency(mw1,mw2);
                    }
                }
            }
        }
    }
}


void ExecutionManager::addModuleDependency(std::shared_ptr<ModuleWrapper> dependent, std::shared_ptr<ModuleWrapper> independent){
    cycleList.edge(independent->instance(), dependent->instance());
}


Profiler& ExecutionManager::profiler() {
    return m_profiler;
}

Messaging& ExecutionManager::messaging() {
    return m_messaging;
}

void ExecutionManager::fireConfigsChangedEvent() {
    for(auto mod : enabledModules) {
        mod.second->instance()->configsChanged();
    }
}

int ExecutionManager::cycleCounter() {
    return m_cycleCounter;
}

ExecutionManager::EnableConfig& ExecutionManager::config(std::string const& name) {
    return m_configs[name];
}

bool ExecutionManager::useConfig(std::string const& name) {
    for(ModuleToEnable const& mod : m_configs[name]) {
        if(! enableModule(mod.first, mod.second)) {
            return false;
        }
    }
    return true;
}

void ExecutionManager::writeDAG(DotExporter &dot, const std::string &prefix) {
    cycleList.removeTransitiveEdges();

    for(auto const& pair : cycleList) {
        dot.label(pair.first->getName());
        dot.node(prefix + "_" + pair.first->getName());
    }

    dot.reset();

    for(auto const& pair : cycleList) {
        std::string from = pair.first->getName();

        for(auto const& to : pair.second) {
            dot.edge(prefix + "_" + to->getName(), prefix + "_" + from);
        }
    }
}

WatchDog & ExecutionManager::dog() {
    return m_dog;
}

}  // namespace internal
}  // namespace lms
