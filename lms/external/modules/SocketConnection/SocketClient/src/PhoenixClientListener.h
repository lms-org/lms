#ifndef PHOENIX_CLIENT_LISTENER_H
#define PHOENIX_CLIENT_LISTENER_H

#include <sockets/SocketClientListener.h>
#include <sys/types.h>
#include <string.h>
#include <sockets/SocketDataTypes.h>
#include <sockets/SocketClient.h>
#include "wifi_client.h"

class PhoenixClientListener: public SocketClientListener {

public:
    SocketClient* client;
    Wifi_client* wifi;

void receivedMessage(char* buff, int bytesRead);

void disconnected();

};


#endif /*PHOENIX_CLIENT_LISTENER_H */
