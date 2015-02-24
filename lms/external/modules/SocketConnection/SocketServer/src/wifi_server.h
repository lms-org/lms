#ifndef IMPORTER_WIFI_SERVER_IMPORTER_H
#define IMPORTER_WIFI_SERVER_IMPORTER_H

#include <core/datamanager.h>
#include <sockets/SocketServer.h>

class Wifi_Server{

    SocketServer* server;
public:
	bool initialize();
	bool deinitialize();

	bool cycle();

    char buffer[1000*100];

protected:
};

#endif
