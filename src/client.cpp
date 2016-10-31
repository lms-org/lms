#include "lms/client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#include "lms/exception.h"

namespace lms {

Client::Client(int fd) : m_socket(fd) {}

Client::~Client() { m_socket.close(); }

Client Client::fromUnix(const std::string &path) {
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

    return Client(fd);
}

ProtobufSocket& Client::sock() {
    return m_socket;
}

}  // namespace lms
