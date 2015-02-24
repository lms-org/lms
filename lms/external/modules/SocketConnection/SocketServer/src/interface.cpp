#include "wifi_server.h"

extern "C" {
void* getInstance () {
    return new Wifi_Server();
}
const char* getName() {
    return "Wifi_Server";
}
}
