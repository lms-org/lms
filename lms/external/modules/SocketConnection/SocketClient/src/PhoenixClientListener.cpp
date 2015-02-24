#ifndef PHOENIX_CLIENT_LISTENER_H
#define PHOENIX_CLIENT_LISTENER_H

#include "sockets/SocketClientListener.h"
#include <sys/types.h>
#include <string.h>
#include <sockets/SocketDataTypes.h>
#include <sockets/SocketClient.h>
#include "wifi_client.h"

class PhoenixClientListener: public SocketClientListener {

public:
    SocketClient* client;
    Wifi_Client* wifi;

void receivedMessage(char* buff, int bytesRead);

void disconnected();

};

#include <stdio.h>

void PhoenixClientListener::receivedMessage(char* buff, int bytesRead){
    char* tempBuffer = buff;
 //   printf("received message: %d\n",bytesRead);
    while (bytesRead != 0){
   //     printf("bytesRead: %d\n",bytesRead);
        char type = buff[0];
        u_int32_t size = 0;
        memcpy(&size,&buff[1],4);
        if(size > bytesRead-5){
      //      printf("packet doesnt fit, wait for more data) %d\n",size);
            memcpy(tempBuffer,buff,bytesRead);
            client->index_of_buffer = bytesRead;
            return;
        }
     //   printf("size to read %d\n",size);

/*
        typedef void (*ParseFn) (char*, int,Wifi_client*);
        ParseFn fnPointers[6]={parseCommand,parseText, parseImg,parseEnvironmentRoad,parseTrajectory};
        fnPointers[type](&buff[5],size,wifi);
        buff = &buff[5+size];
        bytesRead -= 5+size;
        */
    }
    client->index_of_buffer = 0;

}

void PhoenixClientListener::disconnected(){
    printf("client disconnected!\n");
}


#endif /*PHOENIX_CLIENT_LISTENER_H */
