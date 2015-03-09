#include <core/signalhandler.h>
#include <iostream>

namespace lms{
SignalHandler SignalHandler::instance;

SignalHandler& SignalHandler::getInstance() {
    return instance;
}

//TODO https://msdn.microsoft.com/de-de/library/xdkz3x12.aspx
SignalHandler& SignalHandler::addListener(int signalCode, Listener *listener) {
    // the first time we add a listener to a
    // specific signal we must register a signal handler.
    if(listeners[signalCode].empty()) {
        platform_registerSignal(signalCode);
    }
    listeners[signalCode].push_back(listener);

    return *this;
}

SignalHandler& SignalHandler::removeListener(int signalCode, Listener *listener) {
    std::vector<Listener*>& listenerList = listeners[signalCode];

    if(! listenerList.empty()) { // if listenerList is already empty then we don't need to do anything
        for(auto it = listenerList.begin(); it != listenerList.end(); it++) {
            if(*it == listener) {
                listenerList.erase(it);
                break;
            }
        }

        // set signal handling to default if the last
        // listener was removed
        if(listenerList.empty()) {
            platform_unregisterSignal(signalCode);
        }
    }

    return *this;
}

void SignalHandler::handlerForAllSignals(int signal) {
    std::cout << "Signal " << signal << " was triggered" << std::endl;

    for(auto& listener : SignalHandler::getInstance().listeners[signal]) {
        listener->signal(signal);
    }
}

void SignalHandler::platform_registerSignal(int signalCode) {
#ifdef _WIN32
    std::cout << "Signalhandler is not yet implemented on Windows." << std::endl;
#else
    std::cout << "Register listener for " << signalCode << std::endl;
    signal(signalCode, handlerForAllSignals);

    // initialize signal action struct
    //  struct sigaction action;
    //  action.sa_handler = handlerForAllSignals;
    //  sigemptyset(&action.sa_mask);
    //  action.sa_flags = SA_RESTART;
    //  sigaction(signalCode, &action, NULL);
#endif
}

void SignalHandler::platform_unregisterSignal(int signalCode) {
#ifdef _WIN32
    std::cout << "Signalhandler is not yet implemented on Windows." << std::endl;
#else
    //TODO Not sure if we really should set it to the DFL-one. Comment needed!
    std::cout << "Unregister listener for " << signalCode << std::endl;
    signal(signalCode, SIG_DFL);
#endif
}


}
