/**
 * TODO: sende methode schreiben
 *
 */

#include "wifi_server.h"
#include <core/configurationmanager.h>
#include <sockets/SocketServer.h>
#include <sockets/SocketDataTypes.h>


bool Wifi_Server::initialize() {
    printf("Init: wifi_server\n");
    //Set up server
    server = new SocketServer(51720);
    server->start();
    //TODO
	return true;
}

bool Wifi_Server::deinitialize() {
	printf("Deinit: wifi_server");
    server->dispose();

	return true;
}

bool Wifi_Server::cycle () {
    u_int32_t size=0;
    //check for new messages/clients
    printf("cycle.,,,\n");
    server->cycle();
	return true;
}
