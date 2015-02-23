#ifndef SOCKET_CLIENT_LISTENER_H
#define SOCKET_CLIENT_LISTENER_H
class SocketClientListener {

public:
virtual ~SocketClientListener() {}

virtual void receivedMessage(char* buff, int bytesRead)=0;

virtual void disconnected() = 0;

};

#endif /*SOCKET_CLIENT_LISTENER_H */
