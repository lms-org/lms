#include "wifi_client.h"

extern "C" {
void* getInstance () {
    return new Wifi_Client();
}
const char* getName() {
    return "Wifi_Client";
}
}
