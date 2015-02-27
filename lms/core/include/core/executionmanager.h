#ifndef SHARED_EXECUTIONMANAGER_H
#define SHARED_EXECUTIONMANAGER_H

#include "module.h"
#include <string>
#include <deque>
#include <map>
#include <sys/time.h>
#include <core/loader.h>

namespace lms{
    class DataManager;
    class Loader;
    #define MEASURE_TIME 1


    class ExecutionManager {
        //Für was das?
        ExecutionManager& operator = (const ExecutionManager &) = delete;
    public:

        ExecutionManager(DataManager* dataManager);
        virtual ~ExecutionManager();

        /**cycle modues */
        void loop();

        /**Searches the programm for all availabe modules and adds them to the availabe list */
        void loadAvailabelModules();
        /**Enable module with the given name, add it to the cycle-queue */
        void enableModule(std::string name);
        /**Disable module with the given name, remove it from the cycle-queue */
        void disableModule(std::string name);


        void invalidate();
        /**
         * @brief validate if invalidate was called before, it will check if all selected modules are initialised
         * Sorts the modules in the cycle-list
        */
        void validate();
    private:

        bool valid;

        Loader *loader;
        DataManager* dataManager;

        //cycle list
        std::vector<Module*> enabledModules;

        //list of all available modules
        Loader::moduleList available;

    };
}
#endif
