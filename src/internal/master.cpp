#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "lms/internal/master.h"
#include "lms/exception.h"
#include "lms/definitions.h"
#include "tclap/CmdLine.h"

namespace lms {
namespace internal {

std::string getPeer(sockaddr_storage &addr) {
    char ipstr[255];
    int port;

    if (addr.ss_family == AF_UNIX) {
        return "unix";
    } else if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
        return std::string(ipstr) + ":" + std::to_string(port);
    } else if (addr.ss_family == AF_INET6) { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
        return std::string(ipstr) + ":" + std::to_string(port);
    } else {
        return "unknown";
    }
}

void enableReuseAddr(int sock) {
    int mode = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &mode, sizeof(mode)) == -1) {
        perror("setsockopt.SO_REUSEADDR");
    }
}

MasterServer::Client::Client(int fd, const std::string &peer)
    : fd(fd), reader(fd), writer(fd), peer(peer) {}

MasterServer::Server::Server(int fd) : fd(fd) {}

MasterServer::MasterServer() : m_running(true) {}

int MasterServer::useUnix(const std::string &path) {
    int sockfd;

    // Create UNIX Domain socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        return errno;
    }

    // Bind the socket to the given path
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    unlink(path.c_str());

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return errno;
    }

    // Start listening for clients
    if (listen(sockfd, 1) == -1) {
        return errno;
    }

    m_servers.push_back(sockfd);
    return 0;
}

int MasterServer::useIPv4(int port) {
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return errno;
    }

    enableReuseAddr(sockfd);

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return errno;
    }

    // Start listening for clients
    if (listen(sockfd, 1) == -1) {
        return errno;
    }

    m_servers.push_back(sockfd);
    return 0;
}

void MasterServer::enableNonBlock(int sock) {
    int flags = fcntl(sock, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sock, F_SETFL, flags);
}

void MasterServer::start() {
    fd_set fds;
    while (m_running) {
        FD_ZERO(&fds);
        int maxFD = 0;
        for (const Server &server : m_servers) {
            FD_SET(server.fd, &fds);
            maxFD = std::max(maxFD, server.fd);
        }
        for (const Client &client : m_clients) {
            FD_SET(client.fd, &fds);
            maxFD = std::max(maxFD, client.fd);
        }

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(maxFD + 1, &fds, nullptr, nullptr, &timeout);

        if (ret == -1) {
            // TODO error
        }

        for (const Server &server : m_servers) {
            if (FD_ISSET(server.fd, &fds)) {
                sockaddr_storage peer;
                socklen_t peerLen = sizeof peer;
                int clientfd = accept(server.fd, (sockaddr *)&peer, &peerLen);
                enableNonBlock(clientfd);
                m_clients.push_back(Client(clientfd, getPeer(peer)));
            }
        }
        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
            auto &client = *it;
            if (FD_ISSET(client.fd, &fds)) {
                std::string line;
                bool hadLines = false;
                while (client.reader.readLine(line)) {
                    hadLines = true;
                    if (line[line.length() - 1] == '\r') {
                        line = line.substr(0, line.length() - 1);
                    }
                    processClient(client, line);
                }

                if (!hadLines) {
                    // disconnected
                    m_clients.erase(it);
                    --it;
                }
            }
        }
    }
}

void MasterServer::processClient(Client &client, const std::string &message) {
    if (message == "clients") {
        for (const Client &cl : m_clients) {
            client.writer.writeLine(std::to_string(cl.fd) + "\t" + cl.peer);
        }
        client.writer.writeLine();
    } else if (message == "info") {
        client.writer.writeLine("LMS " LMS_VERSION_STRING);
        client.writer.writeLine();
    } else if (message == "pid") {
        client.writer.writeLine(std::to_string(getpid()));
        client.writer.writeLine();
    } else if (message == "shutdown") {
        for (Client &cl : m_clients) {
            cl.writer.writeLine("Shutdown, initiated by " + client.peer);
            cl.writer.writeLine();
        }
        m_running = false;
    } else if (message == "tcpip") {
        int err = useIPv4(3344);
        client.writer.writeLine(std::string("TCPIP Server at ") +
                                std::to_string(err));
    } else if (message == "run") {
    }
}

MasterClient::MasterClient(int fd) : m_sockfd(fd) {}
MasterClient::~MasterClient() { close(m_sockfd); }

MasterClient MasterClient::fromUnix(const std::string &path) {
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

    return MasterClient(fd);
}

int MasterClient::fd() const { return m_sockfd; }

void readUntilEnd(lms::internal::LineReader &reader) {
    std::string line;
    while (reader.readLine(line)) {
        if (line.empty())
            return;
        std::cout << line << std::endl;
    }
}

void connectToMaster(int argc, char *argv[]) {
    TCLAP::CmdLine cmd("LMS Client", ' ', LMS_VERSION_STRING);
    TCLAP::SwitchArg pidSwitch("", "pid", "Get process ID of master server",
                               cmd);
    TCLAP::SwitchArg infoSwitch("", "info", "Get version of master server",
                                cmd);
    TCLAP::SwitchArg connSwitch(
        "", "clients", "Get all clients connected to master server", cmd);
    TCLAP::SwitchArg shutdownSwitch("", "shutdown", "Shutdown master server",
                                    cmd);
    TCLAP::ValueArg<std::string> runArg("r", "run",
                                        "Start a runtime with config", false,
                                        "", "configfile", cmd);
    cmd.parse(argc, argv);

    lms::internal::MasterClient client =
        lms::internal::MasterClient::fromUnix("/tmp/lms.sock");
    lms::internal::LineReader reader(client.fd());
    lms::internal::LineWriter writer(client.fd());

    if (shutdownSwitch.getValue()) {
        writer.writeLine("shutdown");
    } else if (pidSwitch.getValue()) {
        writer.writeLine("pid");
        readUntilEnd(reader);
    } else if (infoSwitch.getValue()) {
        writer.writeLine("info");
        readUntilEnd(reader);
    } else if (connSwitch.getValue()) {
        writer.writeLine("clients");
        readUntilEnd(reader);
    } else if (runArg.isSet()) {
        writer.writeLine("run " + runArg.getValue());
    }
}

/*void interactive() {
    fd_set fds;
    while(true) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(client.fd(), &fds);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int ret = select(client.fd() + 1, &fds, nullptr, nullptr, &timeout);

        if(ret < 0) {
            exit(EXIT_FAILURE);
        }

        if(FD_ISSET(STDIN_FILENO, &fds)) {
            std::string line;
            std::getline(std::cin, line);
            writer.writeLine(line);
        }

        if(FD_ISSET(client.fd(), &fds)) {
            std::string line;
            reader.readLine(line);
            std::cout << line << std::endl;
        }
    }
}*/

} // namespace internal
} // namespace lms
