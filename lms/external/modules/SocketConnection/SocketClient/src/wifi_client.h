#ifndef IMPORTER_WIFI_CLIENT_IMPORTER_H
#define IMPORTER_WIFI_CLIENT_IMPORTER_H

#include <sockets/SocketClient.h>

class Wifi_Client{
    bool first_run;
public:
	bool initialize();
	bool deinitialize();

	bool cycle();


protected: 
    SocketClient* client;
};

#endif
