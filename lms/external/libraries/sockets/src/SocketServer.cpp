/**
 * Really basic SocketServer
 * TODO:
 * add remove(client) function
 * listeners for events (messages/char[], connecting users etc)
 * send message to all users except some
 *
 * code build on top of:
 *  http://www.linuxhowtos.org/C_C++/socket.htm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <sys/select.h>
#include <arpa/inet.h>    //close
#include "sockets/SocketServer.h"

#define TRUE   1
/*
 * http://linux.die.net/man/3/fd_set
 */
SocketServer::SocketServer(int port) {
	SocketServer::port = port;
	struct sockaddr_in temp;
	adress_length = sizeof(temp);
    master_socket = 0;
    bzero(&timeout,sizeof(timeout));
}

void SocketServer::start() {
	struct sockaddr_in serv_addr;
	/*
	 *creates a new socket (TCP as SOCK_STREAM is used)
	 */
	master_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (master_socket < 0)
		error("ERROR opening socket");

	int opt = TRUE;
	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
			sizeof(opt)) < 0) {
		perror("setsockopt");
		exit (EXIT_FAILURE);
	}
	/*
	 * set server_addr to zero
	 */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	/**
	 * set struct values for creating the socket
	 */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	/*convert port numbers*/
	serv_addr.sin_port = htons(port);
	/*bind port to socket */
	if (bind(master_socket, (struct sockaddr *) &serv_addr, adress_length) < 0)
		error("ERROR on binding");
	printf("Listener on port %d \n", port);

	//try to specify maximum of 5 pending connections for the master socket
	if (listen(master_socket, 5) < 0) {
		perror("listen");
		exit (EXIT_FAILURE);
	}
}

void SocketServer::cycle() {
    if(listenToFiles()){
        checkNewConnections();
        checkNewMessages();
    }

}

bool SocketServer::listenToFiles() {
	int max_fd = master_socket;
	FD_ZERO(&fds);
	/**add all clients to the fds */

	//add server file socket (for adding users)
	FD_SET(master_socket, &fds);
    //add users
    printf("listen 1 \n");
	for (std::vector<client>::iterator it = clients.begin();
			it != clients.end(); ++it) {
		struct client & client = *it;
		//if valid socket descriptor then add to read list
		if (client.file > 0) {
			FD_SET(client.file, &fds);
			if(client.file > max_fd){
				max_fd = client.file;
			}
		} else {
			//remove invalid client
			printf("invalid client!!!!");
			it = clients.erase(it) - 1;
		}
	}
    printf("listen 2 \n");
    return select(max_fd+1, &fds, NULL, NULL, &timeout) > 0;
}

void SocketServer::checkNewConnections() {
	//check for new clients
	if (FD_ISSET(master_socket, &fds)) {

		struct client newClient;
		newClient.file = accept(master_socket,
				(struct sockaddr *) &newClient.cli_addr, &adress_length);
		if (newClient.file < 0) {
			error("failed accept");
		}
		addClient(newClient);

	}
}

void SocketServer::checkNewMessages(){
	int n = 0;
	for (std::vector<client>::iterator it = clients.begin();
			it != clients.end(); ++it) {
		struct client & client = *it;
		if (FD_ISSET(client.file, &fds)) {
			n = read(client.file, buffer, 256);
			if (n <= 0) {
				//Somebody disconnected, remove client
				printf("client disconnected \n");
				it = clients.erase(it) - 1;
			} else {
				printf("Server received message: %s\n", buffer);
				char ans[] = "Got your message";
				sendMessageToClient(client, ans, sizeof(ans));
			}
		}
	}
}

void SocketServer::addClient(struct client client) {
	clients.push_back(client);
//	char ans[] = "You can now talk to your master (server)";
//	sendMessageToClient(client, ans, sizeof(ans));
}

void SocketServer::sendMessageToClient(struct client& client,
		const void *buffer, int bytesToSend) {
	int n = write(client.file, buffer, bytesToSend);
	if (n < 0) {
		perror("sendMessage");
	}
}

void SocketServer::sendMessageToAllClients(const void *buffer, int bytesToSend){
    for(auto client:clients){
        sendMessageToClient(client,buffer,bytesToSend);
    }
}

void SocketServer::dispose(){
    //TODO: send bye-message to clients
    close(master_socket);
    for(auto client:clients){
        close(client.file);
    }
}

void SocketServer::error(const char *msg) {
	perror(msg);
	exit(1);
}
