#include <core/signalhandler.h>

namespace lms{
SignalHandler SignalHandler::instance;

SignalHandler& SignalHandler::getInstance() {
    return instance;
}

SignalHandler& SignalHandler::addListener(int signalCode, Listener *listener) {
    // the first time we add a listener to a
    // specific signal we must do the syscall.
    if(listeners[signalCode].empty()) {
        // initialize signal action struct
        struct sigaction action;
        action.sa_handler = handlerForAllSignals;
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_RESTART;
        sigaction(signalCode, &action, NULL);
    }

    listeners[signalCode].push_back(listener);

    return *this;
}

SignalHandler& SignalHandler::removeListener(int signalCode, Listener *listener) {
    std::vector<Listener*>& listenerList = listeners[signalCode];

    for(auto it = listenerList.begin(); it != listenerList.end(); it++) {
        if(*it == listener) {
            listenerList.erase(it);
            break;
        }
    }

    // set signal handling to default if the last
    // listener was removed
    if(listenerList.empty()) {
        signal(signalCode, SIG_DFL);
    }

    return *this;
}

void SignalHandler::handlerForAllSignals(int signal) {
    for(auto& listener : SignalHandler::getInstance().listeners[signal]) {
        listener->signal(signal);
    }
}
}
