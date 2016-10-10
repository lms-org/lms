#pragma once

#include <google/protobuf/message.h>

namespace lms {
namespace internal {

class ProtobufSocket {
public:
    ProtobufSocket(int fd);
    bool writeMessage(const google::protobuf::Message &message);
    bool readMessage(google::protobuf::Message &message);
private:
    int fd;
};

}  // namespace internal
}  // namespace lms
