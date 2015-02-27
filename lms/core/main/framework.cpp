#include <core/framework.h>
#include <core/executionmanager.h>

#include "backtrace_formatter.h"

namespace lms{
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
    logger->info() << "Removing Signal listeners";
    SignalHandler::getInstance()
        .removeListener(SIGINT, this)
        .removeListener(SIGSEGV, this)
        .removeListener(SIGUSR1, this)
        .removeListener(SIGUSR2, this);

    logger->info() << "Killing EXECMGR";
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

            logger->warn() << "Terminating after next Cycle. Press CTRL+C again to terminate immediately";

            SignalHandler::getInstance().removeListener(SIGINT, this);

            break;
        case SIGSEGV:
            //Segmentation Fault - try to identify what went wrong;
            logger->error()
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
