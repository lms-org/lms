#include <core/framework.h>
#include <core/executionmanager.h>

#include <backtrace_formatter.h>

Framework::Framework(int argc, char* const*argv) {

    // TODO überarbeiten für dynamisches modulladen
    argumentHandler = new ArgumentHandler();
    argumentHandler->parse_arguments(argc,argv);


        SignalHandler::getInstance()
            .addListener(SIGINT, this)
            .addListener(SIGSEGV, this)
            .addListener(SIGUSR1, this)
            .addListener(SIGUSR2, this);

        //Execution

        while(running) {
    //		usleep(9000);
            executionManager->loop();
        }
        delete executionManager;

}

void Framework::initManagers(){
    dataManager = new DataManager();
    executionManager = new ExecutionManager();
}

Framework::~Framework() {
    printf("Killing DATAMGR\n");
    delete dataManager;
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
