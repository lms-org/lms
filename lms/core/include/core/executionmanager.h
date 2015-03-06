#ifndef LMS_SHARED_EXECUTIONMANAGER_H
#define LMS_SHARED_EXECUTIONMANAGER_H

#include "module.h"
#include <string>
#include <deque>
#include <map>
#include <vector>
#include <memory>

#include <sys/time.h>
#include <core/loader.h>
#include <core/datamanager.h>
#include <core/executionmanager.h>
#include <core/logger.h>

namespace lms{
    class DataManager;
    class Loader;
    #define MEASURE_TIME 1


    class ExecutionManager {
    public:

        ExecutionManager(logging::Logger &rootLogger);
        ~ExecutionManager();

        /**cycle modues */
        void loop();

        /**
         * Searches the programm for all availabe modules and adds them to the availabe list
         */
        void loadAvailabelModules();
        /**
         * Enable module with the given name, add it to the cycle-queue
         */
        void enableModule(const std::string &name);
        /**
         * Disable module with the given name, remove it from the cycle-queue
         */
        void disableModule(const std::string &name);


        void invalidate();
        /**
         * @brief validate if invalidate was called before, it will check if all selected modules are initialised
         * Sorts the modules in the cycle-list
         */
        void validate();
    private:
        logging::Logger &rootLogger;
        logging::ChildLogger logger;

        int maxThreads;
        bool valid;

        Loader loader;
        DataManager dataManager;

        /**
         * @brief enabledModules contains all loaded Modules
         */
        std::vector<Module*> enabledModules;
        /**
         * @brief cycleListT the cycleListType
         */
        typedef std::vector<std::vector<Module*>> cycleListType;
        cycleListType cycleList;
        /**
         * @brief available contains all Modules which can be loaded
         */
        Loader::moduleList available;
        /**
         * @brief sort call this method for sorting the cycleList
         * As this method isn't called often I won't care about performance but readability. If you have to call this method often you might have to improve it.
         */
        void sort();
        /**
         * @brief sortByDataChannel Sorts enableModules into cycleList. Call this method before sortByPriority().
         * As this method isn't called often I won't care about performance but readability. If you have to call this method often you might have to improve it.
         */
        void sortByDataChannel();
        /**
         * @brief sortByDataChannel Sorts loadedModules into cycleList
         * As this method isn't called often I won't care about performance but readability. If you have to call this method often you might have to improve it.
         */
        void sortByPriority();

    };
}
#endif
