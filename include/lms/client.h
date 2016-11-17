#pragma once

#include <string>
#include <memory>
#include "protobuf_socket.h"

namespace lms {

class Client {
public:
    Client();
    ~Client();
    void connectUnix(const std::string &path);
    ProtobufSocket &sock();

private:
    std::unique_ptr<lms::ProtobufSocket> m_socket;
};

}  // namespace lms
