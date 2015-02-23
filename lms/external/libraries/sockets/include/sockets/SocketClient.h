#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H
#include "SocketClientListener.h"
#include <aio.h>
#include <netinet/in.h>
class SocketClient {

#define BUFFER_SIZE 1000*100

public:
	SocketClient(SocketClientListener* listener);
    char buffer[BUFFER_SIZE];


private:

	/**
	 * file descriptor
	 */
	int sockfd;
	fd_set fds;
	bool connected;

    timeval timeout;
	/**
	*
	*/
	SocketClientListener* listener;
	/**
	 * port of the server
	 */
	int port;

	/**
	 * stores the size of the address of the client, used to accept calls
	 */
	socklen_t adress_length;

	/**
	 * defined in netinet/in.h
	 */
	struct sockaddr_in serv_addr;
	void listenToFiles();
	void checkNewMessages();
	void error(const char *msg);

public:

	void connectToServer(char* address,int port);
	void cycle();
    void sendMessage(int sockfd,const void *buffer, int bytesToSend);
    /**
     * @brief sendMessage Die Reihenfolge ist wichtig von pointers und sizeOfObjects pointers[i] <-> sizeOfObjects[i]
     * @param sockfd file descriptor
     * @param pointers pointing to all objects that will be send
     * @param sizeOfObjects size of each object
     * @param numberOfObjects
     */
   // void sendMessage(int sockfd,int* pointers, int*sizeOfObjects, int numberOfObjects);
	void sendMessage(const void *buffer, int bytesToSend);

    /**
     * @brief index_of_buffer, first free byte in the buffer
     */
    int index_of_buffer;
};

#endif /*SOCKET_CLIENT_H */
