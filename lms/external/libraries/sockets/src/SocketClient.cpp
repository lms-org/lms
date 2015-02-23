#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sockets/SocketClient.h>

SocketClient::SocketClient(SocketClientListener* listener){
	connected = false;
	SocketClient::listener = listener;
    bzero(&timeout,sizeof(timeout));
    index_of_buffer = 0;
}

void SocketClient::error(const char *msg) {
	perror(msg);
	exit(0);
}

void SocketClient::connectToServer(char address[], int port){
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket\n");
	server = gethostbyname(address);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting\n");
	printf("client connected to server\n");
	connected = true;
}

void SocketClient::cycle(){
	if(connected){
		listenToFiles();
		checkNewMessages();
	}else{
		//you are fucked
		listener->disconnected();
		close(sockfd);
		error("not connected\n");
	}
}
void SocketClient::listenToFiles(){
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

    select(sockfd +1, &fds, NULL, NULL, &timeout);
}
void SocketClient::checkNewMessages(){
	int n;
	if (FD_ISSET(sockfd, &fds)) {
            n = read(sockfd, &buffer[index_of_buffer], BUFFER_SIZE-index_of_buffer);
			if (n <= 0) {
				//No new messages from server
				connected = false;
			} else {
                listener->receivedMessage(buffer,n+index_of_buffer);
			}
		}
}
void SocketClient::sendMessage(const void *buffer, int bytesToSend) {
	if(connected){
		int n = write(sockfd, buffer, bytesToSend);
		if (n < 0) {
			perror("sendMessage\n");
		}
	}else{
		//fucked again
		error("not connected (sendMessage)\n");
	}
}
    /**
     * @brief SocketClient::sendMessage
     * @param sockfd
     * @param buffer
     * @param bytesToSend
     */
    void SocketClient::sendMessage(int sockfd,const void *buffer, int bytesToSend) {
        if(connected){
            int n = write(sockfd, buffer, bytesToSend);
            if (n < 0) {
                perror("sendMessage\n");
            }
        }else{
            //fucked again
            error("not connected (sendMessage)\n");
        }
}
