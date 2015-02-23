/**
 * TODO: sende methode schreiben
 *
 */

#include <wifi_server.h>
#include <extra/configurationmanager.h>
#include <SocketServer.h>
#include <data/SocketDataTypes.h>


bool Wifi_server::initialize() {
    printf("Init: wifi_server\n");
    //Set up server
    server = new SocketServer(51720);
    server->start();
	//TODO
    //Get data-channels
    //img_raw

    handle_raw = datamanager()->acquire_channel<unsigned char*>("IMAGE_RAW", Access::READ);

    //umfeldmodell
    handleEnvironmentRoad = datamanager()->acquire_channel<Data::Environment::EnvironmentRoad>("ENVIRONMENT_ROAD", Access::READ);
    //regelpunkt

    handleTrajectory = datamanager()->acquire_channel<Data::Trajectory>("TRAJECTORY", Access::READ);
	return true;
}

bool Wifi_server::deinitialize() {
	printf("Deinit: wifi_server");
    server->dispose();

	return true;
}

bool Wifi_server::cycle () {
    u_int32_t size=0;
    //check for new messages/clients
    printf("cycle.,,,\n");
    server->cycle();

    //schicke Bildpart
    size = 320*240; //TODO: Nicht schoen!
    buffer[0] =  SOCKET_IMG;
    memcpy(&buffer[1],&size,4);
    printf("0. send bytes %d: ",size+5);
    memcpy(&buffer[5],handle_raw->get(),size);
    server->sendMessageToAllClients(buffer,size+5);

    //schicke Umfeldmodell
     size = sizeof(*(handleEnvironmentRoad->get()));
     buffer[0] =  SOCKET_ENVIRONMENT_ROAD;
     memcpy(&buffer[1],&size,4);
     printf("1. send bytes %d: ",size+5);
     memcpy(&buffer[5],handleEnvironmentRoad->get(),size);
     server->sendMessageToAllClients(buffer,size+5);

    //schicke Regelpunkt
     size = sizeof(*(handleTrajectory->get()));
     printf("2. send bytes %d: ",size +5);
     buffer[0] =  SOCKET_TRAJECTORY;
     memcpy(&buffer[1],&size,4);
     memcpy(&buffer[5],handleTrajectory->get(),size);
     server->sendMessageToAllClients(buffer,size+5);


	return true;
}
