#include <core/framework.h>
#include <core/executionmanager.h>

#include "backtrace_formatter.h"

namespace lms{
std::string Framework::programmDirectory;
Framework::Framework(const ArgumentHandler &arguments) : argumentHandler(arguments) {

    initManagers();

    SignalHandler::getInstance()
        .addListener(SIGINT, this)
        .addListener(SIGSEGV, this);
        //TODO Doesnt work on windows
       // .addListener(SIGUSR1, this)
       // .addListener(SIGUSR2, this);
    //load all Availabel Modules
    executionManager->loadAvailabelModules();

    //Execution
    running = true;

    while(running) {
        executionManager->loop();
        //usleep(9000*1000);
    }
}

void Framework::initManagers() {
    dataManager = new DataManager();
    executionManager = new ExecutionManager(dataManager);
}

Framework::~Framework() {
    logger.info() << "Removing Signal listeners";
    SignalHandler::getInstance()
        .removeListener(SIGINT, this)
        .removeListener(SIGSEGV, this);

        //TODO Doesnt work on windows
       // .removeListener(SIGUSR1, this)
       // .removeListener(SIGUSR2, this);

    logger.info() << "Killing EXECMGR";
    //printf("Killing EXECMGR\n");
    delete executionManager;
    logger.info() << "Killing DATAMGR";
    //printf("Killing DATAMGR\n");
    delete dataManager;
}

void Framework::signal(int s) {
    switch (s) {
        case SIGINT:
            running = false;

            logger.warn() << "Terminating after next Cycle. Press CTRL+C again to terminate immediately";

            SignalHandler::getInstance().removeListener(SIGINT, this);

            break;
        case SIGSEGV:
            //Segmentation Fault - try to identify what went wrong;
            logger.error()
                << "######################################################" << std::endl
                << "                   Segfault Found                     " << std::endl
                << "######################################################";

            //In Case of Segfault while recovering - shutdown.
            SignalHandler::getInstance().removeListener(SIGSEGV, this);

            BacktraceFormatter::print();
            break;
    }
}
}
