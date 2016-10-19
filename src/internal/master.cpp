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

#include "messages.pb.h"
#include "protobuf_socket.h"
#include "protobuf_sink.h"

#include "master.h"
#include "lms/exception.h"
#include "lms/definitions.h"
#include "string.h"
#include "tclap/CmdLine.h"
#include "framework.h"

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
    : sock(fd), peer(peer), isAttached(false) {}

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
        for (const auto &client : m_clients) {
            FD_SET(client.sock.getFD(), &fds);
            maxFD = std::max(maxFD, client.sock.getFD());
        }
        for(const Runtime &runtime : m_runtimes) {
            FD_SET(runtime.sock.getFD(), &fds);
            maxFD = std::max(runtime.sock.getFD(), maxFD);
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
                //enableNonBlock(clientfd);
                m_clients.push_back(Client(clientfd, getPeer(peer)));
            }
        }
        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
            auto &client = *it;
            if (FD_ISSET(client.sock.getFD(), &fds)) {
                bool hadLines = false;
                bool exit = false;

                lms::Request req;
                bool readRes = client.sock.readMessage(req);

                if(readRes) {
                    ClientResult res = processClient(client, req);
                    if (res == ClientResult::exit) {
                        exit = true;
                    }
                } else {
                    exit = true;
                }

                if(exit) {
                    client.sock.close();
                    m_clients.erase(it);
                    --it;
                }
            }
        }
        for(Runtime &runtime : m_runtimes) {
            if(FD_ISSET(runtime.sock.getFD(), &fds)) {
                for(auto &client : m_clients) {
                    if(client.isAttached && client.attachedRuntime == runtime.pid) {
                        // forward log events to attached clients
                        LogEvent event;
                        runtime.sock.readMessage(event);
                        client.sock.writeMessage(event);
                    }
                }
            }
        }
    }
}

MasterServer::ClientResult
MasterServer::processClient(Client &client, const lms::Request &message) {
    switch(message.content_case()) {
    case lms::Request::kInfo:
        {
        lms::InfoResponse res;
        res.set_version(LMS_VERSION_CODE);
        res.set_pid(getpid());
        client.sock.writeMessage(res);
        }
        break;
    case lms::Request::kListClients:
        {
        lms::ClientListResponse res;
        for (const auto &cl : m_clients) {
            lms::ClientListResponse_Client *client = res.add_clients();
            client->set_fd(cl.sock.getFD());
            client->set_peer(cl.peer);
        }
        client.sock.writeMessage(res);
        }
        break;
    case lms::Request::kListProcesses:
        {
        lms::ProcessListResponse res;
        for(const auto &rt : m_runtimes) {
            lms::ProcessListResponse_Process *process = res.add_processes();
            process->set_pid(rt.pid);
            process->set_config_file(rt.config_file);
        }
        client.sock.writeMessage(res);
        }
        break;
    case lms::Request::kShutdown:
        m_running = false;
        break;
    case lms::Request::kRun:
        runFramework(client, message.run());
        return ClientResult::attached;
        break;
    case lms::Request::kAttach:
        client.isAttached = true;
        client.attachedRuntime = atoi(message.attach().id().c_str());
        return ClientResult::attached;
    case lms::Request::kStop:
        int signal = message.stop().kill() ? SIGKILL : SIGINT;
        kill(atoi(message.stop().id().c_str()), signal);
        break;
    }

    /*} else if (message == "tcpip") {
        int port = args.size() >= 1 ? atoi(args[0].c_str()) : 3344;
        int err = useIPv4(port);
        if (err != 0) {
            client.writer.writeLine("TCPIP server failed to start");
        } else {
            client.writer.writeLine(std::string("TCPIP Server on port ") +
                                    std::to_string(port));
        }
        client.writer.writeLine();
    } else if (message == "run") {
        runFramework(args[0]);
        client.writer.writeLine("Started: yeah");
        //client.writer.writeLine();
    } else {
        client.writer.writeLine("Unknown command");
        client.writer.writeLine();
    }*/

    return ClientResult::exit;
}

void MasterServer::runFramework(Client &client, const Request_Run &options) {
    pid_t childpid;
    int fd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    // FORK
    if((childpid = fork()) == -1) {
        printf("Fork failed");
        return;
    }

    if(childpid == 0) {
        // init framework

        // close other end of socket
        close(fd[0]);

        logging::Context &ctx = logging::Context::getDefault();
        ctx.appendSink(new ProtobufSink(fd[1]));

        lms::internal::Framework fw(options.config_file());
        for(int i = 0; i < options.include_paths_size(); i++) {
            fw.addSearchPath(options.include_paths(i));
        }
        fw.start();
        exit(0);
    } else {
        // master server

        // close writing ends of pipes
        close(fd[1]);

        Runtime rt{childpid, fd[0], options.config_file()};
        client.isAttached = true;
        client.attachedRuntime = childpid;
        m_runtimes.push_back(rt);
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

void connectToMaster(int argc, char *argv[]) {
    lms::internal::MasterClient client =
        lms::internal::MasterClient::fromUnix("/tmp/lms.sock");
    lms::internal::ProtobufSocket socket(client.fd());

    lms::Request req;

    if(argc >= 2) {
        if(strcmp(argv[1], "version") == 0) {
            req.mutable_info();
            socket.writeMessage(req);

            lms::InfoResponse res;
            socket.readMessage(res);
            std::cout << "Client: " << LMS_VERSION_STRING << "\n";
            std::cout << "Server: " << versionCodeToString(res.version()) << "\n";
        } else if(strcmp(argv[1], "pid") == 0) {
            req.mutable_info();
            socket.writeMessage(req);

            lms::InfoResponse res;
            socket.readMessage(res);
            std::cout << res.pid() << std::endl;
        } else if(strcmp(argv[1], "clients") == 0) {
            req.mutable_list_clients();
            socket.writeMessage(req);

            lms::ClientListResponse res;
            socket.readMessage(res);
            std::cout << "FD \tPEER\n";
            for(int i = 0; i < res.clients_size(); i++) {
                std::cout << res.clients(i).fd() << " \t" << res.clients(i).peer() << "\n";
            }
        } else if(strcmp(argv[1], "ps") == 0) {
            req.mutable_list_processes();
            socket.writeMessage(req);

            lms::ProcessListResponse res;
            socket.readMessage(res);
            std::cout << "ID \tCONFIG\n";
            for(int i = 0; i < res.processes_size(); i++) {
                std::cout << res.processes(i).pid() << " \t" << res.processes(i).config_file() << "\n";
            }
        } else if(strcmp(argv[1], "shutdown") == 0) {
            req.mutable_shutdown();
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "run") == 0) {
            lms::Request_Run *run = req.mutable_run();

            char *lms_path = std::getenv("LMS_PATH");
            if (lms_path != nullptr && lms_path[0] != '\0') {
                for (auto const &path : split(lms_path, ':')) {
                    *run->add_include_paths() = path;
                }
            }

            if(argc >= 3) {
                run->set_config_file(argv[2]);;
                socket.writeMessage(req);
            } else {
                std::cout << "Requires argument: lms run <configfile>\n";
            }

            LogEvent event;
            while(true) {
                socket.readMessage(event);
                std::cout << event.tag() << " " << event.text() << std::endl;
            }
        } else if(strcmp(argv[1], "attach") == 0) {
            lms::Request_Attach *attach = req.mutable_attach();

            if(argc >= 3) {
                attach->set_id(argv[2]);
                socket.writeMessage(req);
            } else {
                std::cout << "Requires argument: lms attach <id> \n";
            }

            LogEvent event;
            while(true) {
                if(!socket.readMessage(event)) {
                    break;
                }
                std::cout << event.tag() << " " << event.text() << std::endl;
            }
        } else if(strcmp(argv[1], "kill") == 0 || strcmp(argv[1], "stop") == 0) {
            lms::Request_Stop *stop = req.mutable_stop();

            if(argc >= 3) {
                stop->set_id(argv[2]);
                stop->set_kill(strcmp(argv[1], "kill") == 0);
                socket.writeMessage(req);
            } else {
                std::cout << "Requires argument: lms kill <id>\n";
            }
        } else {
            std::cout << "Unknown command\n";
        }
    } else {
        std::cout << "LMS - Help\n\n";
        std::cout << "Commands\n";
        std::cout << "  version - Show version info of client and server\n";
        std::cout << "  pid - Print process id of server\n";
        std::cout << "  clients - List all clients connected to server\n";
        std::cout << "  shutdown - Shutdown server\n";
        std::cout << "  run <file> - Start runtime using XML config file\n";
        std::cout << "  ps - List all running runtimes\n";
        std::cout << "  attach <id> - Attach to running runtime\n";
        std::cout << "  kill <id> - Kill runtime (SIGKILL)\n";
        std::cout << "  stop <id> - Stop runtime (SIGINT)\n";
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
