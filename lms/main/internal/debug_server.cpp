#include "lms/internal/debug_server.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <fcntl.h>

namespace lms {
namespace internal {

DebugServer::DebugServer() : logger("DebugServer"), m_shutdown(false) {}

DebugServer::~DebugServer() {
    m_shutdown = true;
    if(m_thread.joinable()) {
        // TODO instead of join() in destructor, better use detach()
        m_thread.join();
    }

    for(Client const& client : m_clients) {
        if(close(client.sockfd) == -1) {
            logger.perror("close");
        }
    }

    for(int const& server : m_server) {
        if(close(server) == -1) {
            logger.perror("close");
        }
    }
}

bool DebugServer::useUnix(std::string const& path) {
    int sockfd;

    if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        logger.perror("socket");
        return false;
    }

    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path)-1);

    unlink(path.c_str());

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        logger.perror("bind");
        return false;
    }

    startListening(sockfd);

    m_server.push_back(sockfd);

    return true;
}

bool DebugServer::useIPv4(uint16_t port) {
    int sockfd;

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        logger.perror("socket");
        return false;
    }

    enableReuseAddr(sockfd);

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        logger.perror("bind");
        return false;
    }

    startListening(sockfd);

    m_server.push_back(sockfd);

    return true;
}

bool DebugServer::useIPv6(uint16_t port) {
    int sockfd;

    if( (sockfd = socket(AF_INET6, SOCK_STREAM, 0)) == -1 ) {
        logger.perror("socket");
        return false;
    }

    enableIPv6Only(sockfd);
    enableReuseAddr(sockfd);

    struct sockaddr_in6 addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin6_flowinfo = 0;
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        logger.perror("bind");
        return false;
    }

    startListening(sockfd);

    m_server.push_back(sockfd);

    return true;
}

bool DebugServer::useDualstack(uint16_t port) {
    // Note: & instead of && (Call both methods in any case)
    return useIPv4(port) & useIPv6(port);
}

void DebugServer::enableReuseAddr(int sock) {
    int mode = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &mode, sizeof(mode)) == -1) {
        logger.perror("setsockopt.SO_REUSEADDR");
    }
}

void DebugServer::enableIPv6Only(int sock) {
    int mode = 1;
    if(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &mode, sizeof(mode)) == -1) {
        logger.perror("setsockopt.IPV6_V6ONLY");
    }
}

void DebugServer::startListening(int sock) {
    if(listen(sock, 1) == -1) {
        logger.perror("listen");
    }
}

void DebugServer::enableNonBlock(int sock) {
    int flags = fcntl(sock, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sock, F_SETFL, flags);
}

void DebugServer::process() {
    FD_ZERO(&m_rfds);
    int maxfd = 0;

    for(auto const& server : m_server) {
        FD_SET(server, &m_rfds);
        maxfd = std::max(maxfd, server);
    }

    for(Client const& client : m_clients) {
        if(client.valid) {
            FD_SET(client.sockfd, &m_rfds);
            maxfd = std::max(maxfd, client.sockfd);
        }
    }

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000; // == 10 ms

    // wait that anything happens
    int retval = select(maxfd + 1, &m_rfds, nullptr, nullptr, &timeout);

    if(retval == -1) {
        logger.perror("select");
        return;
    }

    // new clients tried to connect to this server
    for(auto const& server : m_server) {
        if(FD_ISSET(server, &m_rfds)) {
            processServer(server);
        }
    }

    // connected clients sent some data
    for(Client & client : m_clients) {
        if(client.valid && FD_ISSET(client.sockfd, &m_rfds)) {
            processClient(client);
        }
    }
}

void DebugServer::processServer(int server) {
    Client client;
    client.valid = true;
    client.sockfd = accept(server, nullptr, nullptr);

    if(client.sockfd != -1) {
        m_clients.push_back(client);
    }
}

void DebugServer::processClient(Client & client) {
    int retval = read(client.sockfd, client.buffer.data() + client.bufferUsed,
         client.buffer.size() - client.bufferUsed);

    if(retval == -1) {
        logger.perror("read");
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
