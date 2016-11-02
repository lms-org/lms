#pragma once

#include <google/protobuf/message.h>

namespace lms {

class ProtobufSocket {
public:
    enum Error {
        OK, CLOSED, ERROR
    };

    ProtobufSocket(int fd);
    ~ProtobufSocket();

    Error writeMessage(const google::protobuf::Message &message);
    Error readMessage(google::protobuf::Message &message);
    int getFD() const;
    void close();
private:
    int fd;
};

}  // namespace lms
