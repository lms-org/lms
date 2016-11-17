#include "lms/client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#include "lms/exception.h"

namespace lms {

Client::Client() {}

Client::~Client() {
    if(m_socket) {
        m_socket->close();
    }
}

void Client::connectUnix(const std::string &path) {
    int fd;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        LMS_EXCEPTION("Creating unix client socket failed");
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(fd, (sockaddr *)&addr, sizeof addr) == -1) {
        LMS_EXCEPTION("Connecting unix client socket failed");
    }

    this->m_socket.reset(new ProtobufSocket(fd));
}

ProtobufSocket& Client::sock() {
    if(m_socket) {
        return *m_socket.get();
    } else {
        LMS_EXCEPTION("Client is not connected");
    }
}

}  // namespace lms
