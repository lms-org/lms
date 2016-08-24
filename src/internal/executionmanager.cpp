#include <cstdio>
#include <queue>
#include <iostream>
#include <memory>
#include <algorithm>

#include "lms/internal/executionmanager.h"
#include "lms/internal/framework.h"
#include "lms/internal/viz.h"

namespace lms {
namespace internal {

ExecutionManager::ExecutionManager(Profiler &profiler, Framework &runtime)
    : logger("ExecutionManager"), m_numThreads(1),
      m_multithreading(false), valid(false), m_messaging(),
      m_cycleCounter(-1), running(true), m_profiler(profiler),
      m_runtime(runtime) {}

ExecutionManager::~ExecutionManager() {
    stopRunning();
}


void ExecutionManager::loop() {
    // Remove all messages from the message queue
    m_messaging.resetQueue();

    m_cycleCounter++;

    // validate the ExecutionManager
    //validate();

    if (!m_multithreading) {
        for (Module *mod : sortedCycleList) {
            m_dog.beginModule(mod->getName());

            profiler().markBegin(mod->getName());

            if (m_runtime.isDebug()) {
                logger.debug("executeBegin") << mod->getName();
            }

            try {
                mod->cycle();
            } catch (std::exception const &ex) {
                logger.error("cycle") << mod->getName() << " throws "
                                      << lms::typeName(ex) << " : "
                                      << ex.what();
            }

            if (m_runtime.isDebug()) {
                logger.debug("executeEnd") << mod->getName();
            }

            profiler().markEnd(mod->getName());

            m_dog.endModule();
        }
    } else {
        if (m_runtime.isDebug()) {
            logger.info() << "Cycle start";
        }

        // if thread pool is not yet initialized then do it now
        if (threadPool.empty()) {
            for (int threadNum = 1; threadNum <= m_numThreads; threadNum++) {
                threadPool.push_back(std::thread(
                    [threadNum, this]() { threadFunction(threadNum); }));
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

        if (m_runtime.isDebug()) {
            logger.info() << "Cycle end";
        }
    }
}

void ExecutionManager::threadFunction(int threadNum) {
    // Thread function

    std::unique_lock<std::mutex> lck(mutex);

    while (running) {
        // wait until something is in the cycleList
        cv.wait(lck, [this, threadNum]() {
            // the main thread stops
            if (threadNum == 0 && numModulesToExecute == 0) {
                return true;
            }

            return hasExecutableModules(threadNum);
        });

        if (numModulesToExecute == 0) {
            break;
        }

        Module *executableModule;

        bool found =
            cycleListTmp.getFree(executableModule, [threadNum](Module *mod) {
                return (mod->isMainThread() &&
                        threadNum == 0) ||
                       (!mod->isMainThread() &&
                        threadNum != 0);
            });

        if (found) {
            // if an executable module was found
            // then delete it from the cycleListTmp
            cycleListTmp.removeNode(executableModule);

            if (m_runtime.isDebug()) {
                logger.info() << "Thread " << threadNum << " executes "
                              << executableModule->getName();
            }

            // now we can execute it
            lck.unlock();
            profiler().markBegin(executableModule->getName());

            try {
                executableModule->cycle();
            } catch (std::exception const &ex) {
                logger.error("cycle") << executableModule->getName()
                                      << " throws " << lms::typeName(ex)
                                      << " : " << ex.what();
            }
            profiler().markEnd(executableModule->getName());

            lck.lock();

            if (m_runtime.isDebug()) {
                logger.info() << "Thread " << threadNum << " executed "
                              << executableModule->getName();
            }

            // now we should delete the executed module from
            // the dependencies of other modules
            cycleListTmp.removeEdgesFrom(executableModule);

            numModulesToExecute--;

            // now inform our fellow threads that something new
            // can be executed
            cv.notify_all();
        }
    }
}

bool ExecutionManager::hasExecutableModules(int thread) {
    if (!running) {
        return true;
    }

    if (cycleListTmp.empty()) {
        return false;
    }

    return cycleListTmp.hasFree([thread](Module *mod) -> bool {
        return (mod->isMainThread() && thread == 0) ||
               ((!mod->isMainThread() && thread != 0));
    });
}

void ExecutionManager::stopRunning() {
    {
        std::lock_guard<std::mutex> lck(mutex);
        running = false;
        cv.notify_all();
    }

    for (std::thread &th : threadPool) {
        th.join();
    }
}

void ExecutionManager::invalidate() { valid = false; }

void ExecutionManager::validate(const std::map<std::string, std::shared_ptr<Module>> &enabledModules) {
    if (!valid) {
        valid = true;

        logger.debug("sort") << "No. of enabled modules: " << enabledModules.size();

        // add modules to the list
        cycleList = moduleChannelGraph.generateDAG();
        for (auto it : enabledModules) {
            cycleList.node(it.second.get());
        }

        sortedCycleList.clear();
        bool success = cycleList.topoSort(sortedCycleList);

        if (!success) {
            logger.error("validate") << "Module graph has circle";
        }
    }
}

void ExecutionManager::numThreads(int num) { m_numThreads = num; }

void ExecutionManager::numThreadsAuto() {
    m_numThreads = std::thread::hardware_concurrency();
}

int ExecutionManager::numThreads() const { return m_numThreads; }

void ExecutionManager::enabledMultithreading(bool flag) {
    m_multithreading = flag;
}

bool ExecutionManager::enabledMultithreading() const {
    return m_multithreading;
}

void ExecutionManager::printCycleList(DAG<Module *> &clist) {
    clist.removeTransitiveEdges();

    for (auto const &pair : clist) {
        std::string line(pair.first->getName() + " (");

        for (Module *mod : pair.second) {
            line += " " + mod->getName();
        }

        line += " )";

        logger.debug("cycleList") << line;
    }
}

void ExecutionManager::printCycleList() { printCycleList(cycleList); }

Profiler &ExecutionManager::profiler() { return m_profiler; }

Messaging &ExecutionManager::messaging() { return m_messaging; }

int ExecutionManager::cycleCounter() { return m_cycleCounter; }

void ExecutionManager::writeDAG(DotExporter &dot, const std::string &prefix) {
    cycleList.removeTransitiveEdges();
    dumpDAG(cycleList, dot, prefix);
}

WatchDog &ExecutionManager::dog() { return m_dog; }

ModuleChannelGraph<Module *> &ExecutionManager::getModuleChannelGraph() {
    return moduleChannelGraph;
}

} // namespace internal
} // namespace lms
