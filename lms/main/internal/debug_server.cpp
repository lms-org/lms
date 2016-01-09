#include "lms/internal/debug_server.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>

namespace lms {
namespace internal {

DebugServer::DebugServer() : m_shutdown(false) {}

DebugServer::~DebugServer() {
    m_shutdown = true;
    if(m_thread.joinable()) {
        // TODO instead of join() in destructor, better use detach()
        m_thread.join();
    }
}

bool DebugServer::useUnixSocket(std::string const& path) {
    struct sockaddr_un addr;

    if ( (m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path)-1);

    unlink(path.c_str());

    if (bind(m_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        return false;
    }

    if (listen(m_sockfd, 5) == -1) {
        perror("listen error");
        return false;
    }

    return true;
}

void DebugServer::process() {
    FD_ZERO(&m_rfds);

    FD_SET(m_sockfd, &m_rfds);
    int maxfd = m_sockfd;

    for(Client const& client : m_clients) {
        if(client.valid) {
            FD_SET(client.sockfd, &m_rfds);
            if(client.sockfd > maxfd) {
                maxfd = client.sockfd;
            }
        }
    }

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000; // == 10 ms

    // wait that anything happens
    int retval = select(maxfd + 1, &m_rfds, nullptr, nullptr, &timeout);

    if(retval == -1) {
        perror("process");
        return;
    }

    // new clients tried to connect to this server
    if(FD_ISSET(m_sockfd, &m_rfds)) {
        acceptClients();
    }

    // connected clients sent some data
    for(Client & client : m_clients) {
        if(client.valid && FD_ISSET(client.sockfd, &m_rfds)) {
            processClient(client);
        }
    }
}

void DebugServer::acceptClients() {
    Client client;
    client.valid = true;
    client.sockfd = accept(m_sockfd, NULL, NULL);

    if(client.sockfd != -1) {
        m_clients.push_back(client);
    }
}

void DebugServer::processClient(Client & client) {
    int retval = read(client.sockfd, client.buffer.data() + client.bufferUsed,
         client.buffer.size() - client.bufferUsed);

    if(retval == -1) {
        perror("processClient");
        client.valid = false;
    } else if(retval == 0) {
        // connection closed
        client.valid = false;
    } else {
        client.bufferUsed += retval;
    }
}

void DebugServer::broadcast(std::uint8_t type, const Datagram &datagram) {
    std::uint32_t size = htonl(datagram.data.size());

    for(Client const& client : m_clients) {
        if(client.valid) {
            write(client.sockfd, &size, sizeof(size));
            write(client.sockfd, &type, sizeof(type));
            write(client.sockfd, datagram.data.data(), datagram.data.size());
        }
    }
}

void DebugServer::startThread() {
    m_thread = std::thread([this] () {
        while(! m_shutdown) {
            process();
        }
    });
}

}  // namespace internal
}  // namespace lms
