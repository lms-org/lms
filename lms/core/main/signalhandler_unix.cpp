#include <iostream>
#include <csignal>

#include <lms/signalhandler.h>

namespace lms {

void SignalHandler::platform_registerSignal(int signalCode) {
    std::cout << "Register listener for " << signalCode << std::endl;
    signal(signalCode, handlerForAllSignals);

    // initialize signal action struct
//  struct sigaction action;
//  action.sa_handler = handlerForAllSignals;
//  sigemptyset(&action.sa_mask);
//  action.sa_flags = SA_RESTART;
//  sigaction(signalCode, &action, NULL);
}

void SignalHandler::platform_unregisterSignal(int signalCode) {
    //TODO Not sure if we really should set it to the DFL-one. Comment needed!
    std::cout << "Unregister listener for " << signalCode << std::endl;
    signal(signalCode, SIG_DFL);
}

}

