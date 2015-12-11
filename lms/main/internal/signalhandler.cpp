#include <lms/internal/signalhandler.h>
#include <iostream>

namespace lms {
namespace internal {

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
    for(auto& listener : SignalHandler::getInstance().listeners[signal]) {
        listener->signal(signal);
    }
}

}  // namespace internal
}  // namespace lms
