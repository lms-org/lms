#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <aio.h>
#include <netinet/in.h>
class SocketServer {

public:
    SocketServer(int port);
    char buffer[1000*100];

	struct client {
		struct sockaddr_in cli_addr;
		int file;
		/*
		 * buffer for read
		 */
		aiocb cb;
		friend bool operator==(const client& lhs, const client& rhs){
				return lhs.file == rhs.file;
			}
	};

private:

	/**
	 * file descriptor
	 */
	int master_socket;
	fd_set fds;


    timeval timeout;

	/**
	 * port of the server
	 */
	int port;

	std::vector<struct client> clients;

	/**
	 * stores the size of the address of the client, used to accept calls
	 */
	socklen_t adress_length;

	/**
	 * defined in netinet/in.h
	 */
	struct sockaddr_in serv_addr;
    bool listenToFiles();
	void checkNewConnections();
	void checkNewMessages();
    void error(const char *msg);

public:

	void start();
	void addClient(struct client client);
	void sendMessageToClient(struct client& client,const void *buffer,int bytesToSend);
    void sendMessageToAllClients(const void *buffer, int bytesToSend);
	void cycle();
    void dispose();
};

#endif /*SOCKET_SERVER_H */
