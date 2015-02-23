#ifndef IMPORTER_WIFI_SERVER_IMPORTER_H
#define IMPORTER_WIFI_SERVER_IMPORTER_H

#include <core/importer.h>
#include <core/datamanager.h>
#include <SocketServer.h>
#include <data/image_data.h>
#include <data/env/roadfeature.h>
#include <data/env/road.h>
#include <data/trajectory.h>

class Wifi_server : public Importer {
	IMPLEMENT_BASE;

    SocketServer* server;
public:
	bool initialize();
	bool deinitialize();

	bool cycle();

    char buffer[1000*100];

protected:
    //Umfeldmodell
    Handle<Data::Environment::EnvironmentRoad> *handleEnvironmentRoad;
    //Regelpunkt
    Handle<Data::Trajectory> *handleTrajectory;
    //Raw image
    Handle<unsigned char*>* handle_raw;
};

#endif
