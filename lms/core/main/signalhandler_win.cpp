#include <iostream>

#include <core/signalhandler.h>

namespace lms {

void SignalHandler::platform_registerSignal(int signalCode) {
    std::cout << "Signalhandler is not yet implemented on Windows." << std::endl;
}

void SignalHandler::platform_unregisterSignal(int signalCode) {
    std::cout << "Signalhandler is not yet implemented on Windows." << std::endl;
}

}


