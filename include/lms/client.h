#pragma once

#include <string>
#include "protobuf_socket.h"

namespace lms {

class Client {
public:
    static Client fromUnix(const std::string &path);
    ProtobufSocket &sock();
    ~Client();

private:
    lms::ProtobufSocket m_socket;
    Client(int fd);
};

}  // namespace lms
