#include "wifi_client.h"
#include "PhoenixClientListener.h"
#include <core/configurationmanager.h>
#include <sockets/SocketClient.h>

bool Wifi_Client::initialize() {
	printf("Init: wifi_client\n");
    first_run = false;
    PhoenixClientListener* listener = new PhoenixClientListener();
    client = new SocketClient(listener);
    listener->client = client;
    listener->wifi = this;
    client->connectToServer("127.0.0.1",51720);

	return true;
}

bool Wifi_Client::deinitialize() {
	printf("Deinit: wifi_client");
	//TODO
	return true;
}

bool Wifi_Client::cycle () {
    client->cycle();
	return true;
}
