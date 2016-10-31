#pragma once

#include <google/protobuf/message.h>

namespace lms {

class ProtobufSocket {
public:
    ProtobufSocket(int fd);

    bool writeMessage(const google::protobuf::Message &message);
    bool readMessage(google::protobuf::Message &message);
    int getFD() const;
    void close();
private:
    int fd;
};

}  // namespace lms
