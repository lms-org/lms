#include <core/framework.h>
#include <core/executionmanager.h>

#include "backtrace_formatter.h"

Framework::Framework(const ArgumentHandler &arguments) : argumentHandler(arguments) {

    initManagers();

    SignalHandler::getInstance()
        .addListener(SIGINT, this)
        .addListener(SIGSEGV, this)
        .addListener(SIGUSR1, this)
        .addListener(SIGUSR2, this);

    //Execution
    running = true;

    while(running) {
        executionManager->loop();
        //usleep(9000*1000);
    }
}

void Framework::initManagers() {
    sink = new ConsoleSink(std::cout);
    logger = new RootLogger(sink);

    dataManager = new DataManager();
    executionManager = new ExecutionManager();
}

Framework::~Framework() {
    logger->info() << "Killing EXECMGR" << std::endl << "bla";
    //printf("Killing EXECMGR\n");
    delete executionManager;
    logger->info() << "Killing DATAMGR";
    //printf("Killing DATAMGR\n");
    delete dataManager;

    delete logger;
    delete sink;
}

void Framework::signal(int s) {
    switch (s) {
        case SIGINT:
            running = false;
            printf("\n\033[35mTerminating after next Cycle.\033[0m\nPress CTRL+C again to terminate immediately\n");
            SignalHandler::getInstance().removeListener(SIGINT, this);

            break;
        case SIGSEGV:
            //Segmentation Fault - try to identificate what went wrong;
            printf( "\033[34m######################################################\n"
                    "\033[34m#\033[31m   Segfault Found                                   \033[34m#\n"
                    "\033[34m######################################################\033[0m\n");
            //In Case of Segfault while recovering - shutdown.
            SignalHandler::getInstance().removeListener(SIGSEGV, this);

            BacktraceFormatter::print();
            break;
    }
}
