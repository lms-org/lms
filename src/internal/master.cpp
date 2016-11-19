#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cmath>

#include "messages.pb.h"
#include "lms/protobuf_socket.h"
#include "protobuf_sink.h"

#include "master.h"
#include "lms/exception.h"
#include "lms/definitions.h"
#include "string.h"
#include "tclap/CmdLine.h"
#include "framework.h"
#include "colors.h"
#include "lms/client.h"
#include "os.h"


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

/**
 * @brief Collect all zombies (not blocking)
 */
void rickGrimes(int signal) {
    (void)signal; // ignore parameter
    int status;
    while(waitpid(-1, &status, WNOHANG) >= 0) {
        std::cout << "Collected zombie" << std::endl;
    }
}

void MasterServer::start() {
    // Fix broken pipe behavior
    ::signal(SIGPIPE, SIG_IGN);

    // Kill zombie child processes
    ::signal(SIGCHLD, rickGrimes);

    fd_set fds;

    //save cout from master
    //std::ofstream out("/tmp/master.txt",std::trunc);
    //old buffer
    //std::streambuf *coutbuf = std::cout.rdbuf();
    //save it to file
    //std::cout.rdbuf(out.rdbuf());

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
            FD_SET(runtime.logSock.getFD(), &fds);
            maxFD = std::max(runtime.logSock.getFD(), maxFD);
            FD_SET(runtime.commSock.getFD(), &fds);
            maxFD = std::max(runtime.commSock.getFD(), maxFD);
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
                std::cout << "New Client" << std::endl;
                sockaddr_storage peer;
                socklen_t peerLen = sizeof peer;
                int clientfd = accept(server.fd, (sockaddr *)&peer, &peerLen);
                //enableNonBlock(clientfd);
                m_clients.push_back(Client(clientfd, getPeer(peer)));

                // push broadcast
                lms::Response response;
                buildListClientsResponse(response);
                broadcastResponse(response);
            }
        }
        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
            auto &client = *it;
            if (FD_ISSET(client.sock.getFD(), &fds)) {
                std::cout << "Client packet" << std::endl;

                lms::Request req;
                auto readRes = client.sock.readMessage(req);

                if(readRes == ProtobufSocket::OK) {
                    processClient(client, req);
                } else {
                    if(client.isAttached && client.shutdownRuntimeOnDetach) {
                        std::cout << "Kill runtime " << client.attachedRuntime << " because client detached" << std::endl;
                        kill(client.attachedRuntime, SIGINT);
                    }

                    // push broadcast
                    lms::Response response;
                    buildListClientsResponse(response);
                    broadcastResponse(response);

                    pid_t rtPid = client.attachedRuntime;
                    bool wasAttached = client.isAttached;

                    client.sock.close();
                    m_clients.erase(it);
                    --it;

                    if(wasAttached) {
                        Runtime *rt = getRuntimeByPid(rtPid);
                        if(rt != nullptr && !isSomeoneAttached(rtPid)) {
                            setLogLevel(rt->commSock, logging::Level::OFF);
                        }
                    }
                }
            }
        }
        for(auto it = m_runtimes.begin(); it != m_runtimes.end(); ++it) {
            auto &runtime = *it;
            if(FD_ISSET(runtime.logSock.getFD(), &fds)) {
                // forward log events to attached clients
                Response response;
                if(runtime.logSock.readMessage(response) == ProtobufSocket::OK) {
                    for(auto &client : m_clients) {
                        if(client.isAttached && client.attachedRuntime == runtime.pid) {
                            if(!response.has_log_event()
                                || static_cast<logging::Level>(response.log_event().level()) >= client.logLevel) {
                                client.sock.writeMessage(response);
                            }
                        }
                    }
                } else {
                    Response closeEvent;
                    Response::LogEvent *event = closeEvent.mutable_log_event();
                    event->set_level(Response::LogEvent::INFO);
                    event->set_tag("master");
                    event->set_text("Runtime stopped");
                    event->set_close_after(true);
                    for(auto &client : m_clients) {
                        if(client.isAttached && client.attachedRuntime == runtime.pid) {
                             client.sock.writeMessage(closeEvent);
                        }
                    }

                    // push broadcast
                    lms::Response response;
                    buildListRuntimesResponse(response);
                    broadcastResponse(response);

                    // close runtime connection
                    runtime.logSock.close();
                    runtime.commSock.close();
                    m_runtimes.erase(it);
                    it--;
                    std::cout << "Could not read msg from runtime" << std::endl;
                    //TODO error handling
                }
            }
            if(FD_ISSET(runtime.commSock.getFD(), &fds)) {
                Response response;
                if(runtime.commSock.readMessage(response) == ProtobufSocket::OK) {
                    std::cout << "Got message via commSocket from " << runtime.name << "\n";
                    for(auto &client : m_clients) {
                        if(client.isAttached && client.attachedRuntime == runtime.pid) {
                            client.sock.writeMessage(response);
                        }
                    }
                }
            }
        }
    }

    // graceful shutdown
    for(const auto &rt : m_runtimes) {
        std::cout << "Shutdown " << rt.pid << std::endl;
        kill(rt.pid, SIGINT);
    }
}

bool MasterServer::isSomeoneAttached(pid_t id) {
    for(const auto &cl : m_clients) {
        if(cl.isAttached && cl.attachedRuntime == id) {
            return true;
        }
    }
    return false;
}

void MasterServer::broadcastResponse(const Response &response) {
    for(auto &client : m_clients) {
        if(client.listenBroadcats) {
            client.sock.writeMessage(response);
        }
    }
}

void MasterServer::buildListRuntimesResponse(lms::Response &response) {
    auto list = response.mutable_process_list();
    for(const auto &rt : m_runtimes) {
        auto *process = list->add_processes();
        process->set_pid(rt.pid);
        process->set_config_file(rt.config_file);
        process->set_name(rt.name);
    }
}

void MasterServer::buildListClientsResponse(lms::Response &response) {
    auto list = response.mutable_client_list();
    for (const auto &cl : m_clients) {
        auto client = list->add_clients();
        client->set_fd(cl.sock.getFD());
        client->set_peer(cl.peer);
    }
}

void MasterServer::processClient(Client &client, const lms::Request &message) {
    Response response;
    bool sendResponse = true;

    switch(message.content_case()) {
    case lms::Request::kInfo:
        {
        auto info = response.mutable_info();
        info->set_version(LMS_VERSION_CODE);
        info->set_pid(getpid());
        }
        break;
    case lms::Request::kListClients:
        buildListClientsResponse(response);
        break;
    case lms::Request::kListProcesses:
        buildListRuntimesResponse(response);
        break;
    case lms::Request::kShutdown:
        m_running = false;
        break;
    case lms::Request::kRun:
        {
        runFramework(client, message.run());
        // push broadcast
        lms::Response response;
        buildListRuntimesResponse(response);
        broadcastResponse(response);
        }
        break;
    case lms::Request::kAttach:
        {
        Runtime *rt = nullptr;
        if(message.attach().has_name()) {
            rt = getRuntimeByName(message.attach().name());
        } else if(m_runtimes.size() == 1) {
            rt = &m_runtimes[0];
        }
        if(rt != nullptr) {
            client.isAttached = true;
            client.attachedRuntime = rt->pid;
            client.shutdownRuntimeOnDetach = false;
            client.logLevel = static_cast<logging::Level>(message.attach().log_level());
            setLogLevel(rt->commSock, logging::Level::ALL);
        }
        }
        break;
    case lms::Request::kStop:
        {
        Runtime *rt = nullptr;
        if(message.stop().has_name()) {
            rt = getRuntimeByName(message.stop().name());
        } else if(m_runtimes.size() == 1) {
            rt = &m_runtimes[0];
        }
        if(rt != nullptr) {
            int signal = message.stop().kill() ? SIGKILL : SIGINT;
            kill(rt->pid, signal);
        }
        }
        break;
    case lms::Request::kRuntime:
        {
        Runtime *rt = nullptr;
        if(message.runtime().has_name()) {
            rt = getRuntimeByName(message.runtime().name());
        } else if(m_runtimes.size() == 1) {
            rt = &m_runtimes[0];
        }
        if(rt != nullptr) {
            std::cout << "Got kRuntime request\n";
            client.isAttached = true;
            client.attachedRuntime = rt->pid;
            client.shutdownRuntimeOnDetach = false;
            client.logLevel = logging::Level::OFF;
            rt->commSock.writeMessage(message);
            sendResponse = false;
        }
        }
        break;
    case lms::Request::kListenBroadcasts:
        client.listenBroadcats = message.listen_broadcasts().enable();
        break;
    case lms::Request::kDetach:
        {
        bool wasAttached = client.isAttached;
        pid_t rtPid = client.attachedRuntime;

        client.isAttached = false;
        client.shutdownRuntimeOnDetach = false;

        if(wasAttached) {
            Runtime *rt = getRuntimeByPid(rtPid);
            if(rt != nullptr && !isSomeoneAttached(rtPid)) {
                setLogLevel(rt->commSock, logging::Level::OFF);
            }
        }
        }
        break;
    }

    if(sendResponse) {
        client.sock.writeMessage(response);
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
}

void setLogLevel(ProtobufSocket &sock, logging::Level level) {
    lms::Request req;
    req.mutable_runtime()->mutable_filter()->set_log_level(
                static_cast<Response::LogEvent::Level>(level));
    sock.writeMessage(req);
}

void MasterServer::runFramework(Client &client, const Request_Run &options) {
    pid_t childpid;
    int logFd[2];
    int commFd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, logFd); // used in the logger
    socketpair(AF_UNIX, SOCK_STREAM, 0, commFd); // used in communication thread of the framework

    // FORK
    if((childpid = fork()) == -1) {
        printf("Fork failed");
        return;
    }

    if(childpid == 0) {
        // init framework

        // close other end of socket
        close(logFd[0]);
        close(commFd[0]);

        logging::Context &ctx = logging::Context::getDefault();
        logging::Level logLevel = options.production() ? logging::Level::WARN : logging::Level::ALL;
        ctx.appendSink(new ProtobufSink(logFd[1], logLevel));

        if(options.detached()) {
            logging::Context::getDefault().setLevel(logging::Level::OFF);
        }

        lms::internal::Framework fw(options.config_file());
        fw.setDebug(options.debug());
        for(int i = 0; i < options.include_paths_size(); i++) {
            fw.addSearchPath(options.include_paths(i));
        }
        for(int i = 0; i < options.flags_size(); i++) {
            fw.addFlag(options.flags(i));
        }
        if(options.has_load_path()) {
            fw.enableLoad(options.load_path());
        }
        if(options.has_save_path()) {
            fw.enableSave(options.save_path());
        }
        if(options.has_num_threads()) {
            fw.executionManager().enabledMultithreading(true);
            fw.executionManager().numThreads(options.num_threads());
        }
        if(options.has_load_path() && options.load_channels_size() > 0) {
            std::vector<std::string> channels;
            for(int i = 0; i < options.load_channels().size(); i++) {
                channels.push_back(options.load_channels().Get(i));
            }
            fw.loadRecordings(options.load_path(), channels);
        }

        SignalHandler::getInstance().addListener(SIGSEGV, &fw);
        SignalHandler::getInstance().addListener(SIGINT, &fw);

        fw.startCommunicationThread(commFd[1]);
        fw.start();
        exit(0);
    } else {
        // master server

        // close writing ends of pipes
        close(logFd[1]);
        close(commFd[1]);

        std::string name;
        if(options.has_name()) {
            name = options.name();
        } else {
            name = std::to_string(runtimeNameCounter++);
        }
        Runtime rt{name, childpid, logFd[0], commFd[0], options.config_file()};
        if(! options.detached()) {
            client.isAttached = true;
            client.attachedRuntime = childpid;
            client.logLevel = static_cast<logging::Level>(options.log_level());
            client.shutdownRuntimeOnDetach = options.has_shutdown_runtime_on_detach()
                    && options.shutdown_runtime_on_detach();

        }
        m_runtimes.push_back(rt);
    }
}

MasterServer::Runtime* MasterServer::getRuntimeByPid(pid_t id) {
    for(auto &rt : m_runtimes) {
        if(rt.pid == id) {
            return &rt;
        }
    }
    return nullptr;
}

MasterServer::Runtime* MasterServer::getRuntimeByName(const std::string &name) {
    for(auto &rt : m_runtimes) {
        if(rt.name == name) {
            return &rt;
        }
    }
    return nullptr;
}

void streamLogs(ProtobufSocket &socket) {
    Response response;
    while(socket.readMessage(response) == ProtobufSocket::OK) {
        if(!response.has_log_event()) continue;

        const auto &event = response.log_event();
        // get time now
        time_t rawtime = event.timestamp() / 1000 / 1000;
        //std::time(&rawtime);
        struct tm *now = std::localtime(&rawtime);

        // format time to "HH:MM:SS"
        char buffer[10];
        std::strftime(buffer, 10, "%T", now);

        std::cout << buffer << " ";

        std::cout << lms::logging::levelColor(static_cast<lms::logging::Level>(event.level()));
        std::cout << lms::logging::levelName(static_cast<lms::logging::Level>(event.level())) << " " << event.tag();
        std::cout << lms::internal::COLOR_WHITE;
        std::cout << " " << event.text() << std::endl;

        if(event.has_close_after() && event.close_after()) {
            break;
        }
    }
}

void expectResponseType(const Response &response, Response::ContentCase type) {
    if(response.content_case() != type) {
        std::cout << "Unexpected response type: " << response.content_case() << std::endl;
    }
}

std::vector<std::string> logLevels() {
    return std::vector<std::string>{"all", "debug", "info", "warn", "error"};
}

void connectToMaster(int argc, char *argv[]) {
    Client client;
    client.connectUnix("/tmp/lms.sock");
    auto &socket = client.sock();

    lms::Request req;

    if(argc >= 2) {
        if(strcmp(argv[1], "version") == 0) {
            req.mutable_info();
            socket.writeMessage(req);

            Response res;
            socket.readMessage(res);
            expectResponseType(res, Response::kInfo);
            std::cout << "Client: " << LMS_VERSION_STRING << "\n";
            std::cout << "Server: " << versionCodeToString(res.info().version()) << "\n";
        } else if(strcmp(argv[1], "pid") == 0) {
            req.mutable_info();
            socket.writeMessage(req);

            Response res;
            socket.readMessage(res);
            expectResponseType(res, Response::kInfo);
            std::cout << res.info().pid() << std::endl;
        } else if(strcmp(argv[1], "clients") == 0) {
            req.mutable_list_clients();
            socket.writeMessage(req);

            Response res;
            socket.readMessage(res);
            expectResponseType(res, Response::kClientList);
            const auto &list = res.client_list();
            std::cout << "FD \tPEER\n";
            for(int i = 0; i < list.clients_size(); i++) {
                std::cout << list.clients(i).fd() << " \t" << list.clients(i).peer() << "\n";
            }
        } else if(strcmp(argv[1], "ps") == 0) {
            req.mutable_list_processes();
            socket.writeMessage(req);

            Response res;
            socket.readMessage(res);
            expectResponseType(res, Response::kProcessList);
            const auto &list = res.process_list();
            std::cout << "NAME \tID \tCONFIG\n";
            for(int i = 0; i < list.processes_size(); i++) {
                std::cout << list.processes(i).name() << " \t"
                          << list.processes(i).pid() << " \t"
                          << list.processes(i).config_file() << "\n";
            }
        } else if(strcmp(argv[1], "shutdown") == 0) {
            req.mutable_shutdown();
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "run") == 0) {
            auto levels = logLevels();
            TCLAP::ValuesConstraint<std::string> logConstraint(levels);

            TCLAP::CmdLine cmd("lms run", ' ', LMS_VERSION_STRING);
            TCLAP::UnlabeledValueArg<std::string> configArg(
                "config", "XML config path", true, "lms.xml", "XML Config", cmd);
            TCLAP::MultiArg<std::string> loadPathsArg(
                "l", "load-path", "Add additional load path", false, "Path", cmd);
            TCLAP::MultiArg<std::string> flagsArg(
                "f", "flag", "Add flag for XML config", false, "Flag", cmd);
            TCLAP::SwitchArg debugSwitch(
                "", "debug", "Make a ridiculous number of debug outputs", cmd, false);
            TCLAP::ValueArg<std::string> logArg(
                "", "log", "Minimum logging level",
                false, "DEBUG", &logConstraint, cmd);
            TCLAP::SwitchArg detachSwitch(
                "d", "detach", "Start runtime but do not show logging messages", cmd, false);
            TCLAP::SwitchArg shutdownOnDetachSwitch(
                "s", "shutdown-on-detach", "Shutdown runtime when clients disconnects", cmd, false);
            TCLAP::SwitchArg productionSwitch(
                "p", "production", "Enable production mode. This sets the logging level at the runtime to WARN", cmd, false);
            TCLAP::ValueArg<std::string> nameArg(
                "", "name", "Custom runtime name",
                false, "", "NAME", cmd);
            TCLAP::ValueArg<std::string> enableSaveArg(
                "", "enable-save",
                "Enable all saving modules and mark the save folder", false, "", "tag",
                cmd);
            TCLAP::ValueArg<std::string> enableLoadArg(
                "", "enable-load",
                "Enable all loading modules and set a default load path", false, "",
                "path", cmd);
            TCLAP::ValueArg<int> multithreadingArg(
                "j", "", "Enable multithreading and set number of threads",
                false, 4, "NUM", cmd);
            TCLAP::MultiArg<std::string> loadChannelsArg(
                "", "ch", "Add channel to load from", false, "Channel", cmd);
            cmd.parse(argc-1, argv+1);

            lms::Request_Run *run = req.mutable_run();
            run->set_config_file(internal::realpath(configArg.getValue()));
            for(const auto& path : loadPathsArg) {
                *run->add_include_paths() = internal::realpath(path);
            }
            for(const auto& flag : flagsArg) {
                *run->add_flags() = flag;
            }
            run->set_debug(debugSwitch.getValue());
            run->set_detached(detachSwitch.getValue());
            run->set_shutdown_runtime_on_detach(shutdownOnDetachSwitch.getValue());
            run->set_production(productionSwitch.getValue());
            if(nameArg.isSet()) {
                run->set_name(nameArg.getValue());
            }
            logging::Level logLevel = logging::Level::DEBUG;
            if(logging::levelFromName(logArg.getValue(), logLevel)) {
                run->set_log_level(static_cast<lms::Response::LogEvent::Level>(logLevel));
            }
            std::string basepath = lms::internal::homepath() + "/lmslogs";
            if(enableSaveArg.isSet()) {
                ::mkdir(basepath.c_str(), 0775);
                std::string path = basepath + "/" + enableSaveArg.getValue();
                ::mkdir(path.c_str(), 0775);
                run->set_save_path(path);
            }
            if(enableLoadArg.isSet()) {
                ::mkdir(basepath.c_str(), 0775);
                std::string path = enableLoadArg.getValue();
                if(! isAbsolute(path)) {
                    path = basepath + "/" + path;
                }
                run->set_load_path(path);
            }
            if(multithreadingArg.isSet()) {
                run->set_num_threads(multithreadingArg.getValue());
            }
            for(const auto& ch : loadChannelsArg) {
                *run->add_load_channels() = ch;
            }

            char *lms_path = std::getenv("LMS_PATH");
            if (lms_path != nullptr && lms_path[0] != '\0') {
                for (auto const &path : split(lms_path, ':')) {
                    *run->add_include_paths() = path;
                }
            }

            socket.writeMessage(req);

            if(! detachSwitch.getValue()) {
                streamLogs(socket);
            }
        } else if(strcmp(argv[1], "attach") == 0) {
            auto levels = logLevels();
            TCLAP::ValuesConstraint<std::string> logConstraint(levels);
            TCLAP::CmdLine cmd("lms attach", ' ', LMS_VERSION_STRING);
            TCLAP::UnlabeledValueArg<std::string> nameArg(
                "name", "Runtime name", false, "my_runtime", "NAME", cmd);
            TCLAP::ValueArg<std::string> logArg(
                "", "log", "Minimum logging level",
                false, "DEBUG", &logConstraint, cmd);
            cmd.parse(argc-1, argv+1);

            lms::Request_Attach *attach = req.mutable_attach();

            if(nameArg.isSet()) {
                attach->set_name(nameArg.getValue());
            }
            logging::Level logLevel = logging::Level::DEBUG;
            if(logging::levelFromName(logArg.getValue(), logLevel)) {
                attach->set_log_level(static_cast<lms::Response::LogEvent::Level>(logLevel));
            }
            socket.writeMessage(req);

            streamLogs(socket);
        } else if(strcmp(argv[1], "kill") == 0 || strcmp(argv[1], "stop") == 0) {
            lms::Request_Stop *stop = req.mutable_stop();

            if(argc >= 3) {
                stop->set_name(argv[2]);
            }
            stop->set_kill(strcmp(argv[1], "kill") == 0);
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "modules") == 0) {
            lms::Request_ModuleList *modules = req.mutable_module_list();
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "start-recording")) {
            TCLAP::CmdLine cmd("lms start-recording", ' ', LMS_VERSION_STRING);
            TCLAP::UnlabeledValueArg<std::string> nameArg(
                "name", "Runtime name", false, "my_runtime", "NAME", cmd);
            TCLAP::MultiArg<std::string> loadChannelsArg(
                "", "ch", "Add channel to load from", false, "Channel", cmd);
            cmd.parse(argc-1, argv+1);

            lms::Request::Runtime::StartRecording *startRecording =
                    req.mutable_runtime()->mutable_start_recording();

            if(nameArg.isSet()) {
                req.mutable_runtime()->set_name(nameArg.getValue());
            }
            for(const auto& ch : loadChannelsArg) {
                *startRecording->add_channels() = ch;
            }
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "stop-recording")) {
            TCLAP::CmdLine cmd("lms stop-recording", ' ', LMS_VERSION_STRING);
            TCLAP::UnlabeledValueArg<std::string> nameArg(
                "name", "Runtime name", false, "my_runtime", "NAME", cmd);
            TCLAP::ValueArg<std::string> tagArg(
                "", "tag",
                "Set folder name for recorded data", true, "",
                "tag", cmd);
            cmd.parse(argc-1, argv+1);

            lms::Request::Runtime::StopRecording *stopRecording =
                req.mutable_runtime()->mutable_stop_recording();

            if(nameArg.isSet()) {
                req.mutable_runtime()->set_name(nameArg.getValue());
            }
            stopRecording->set_tag(tagArg.getValue());
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "profiling") == 0 || strcmp(argv[1], "prof") == 0) {
            TCLAP::CmdLine cmd("lms profiling", ' ', LMS_VERSION_STRING);
            TCLAP::UnlabeledValueArg<std::string> nameArg(
                "name", "Runtime name", false, "my_runtime", "NAME", cmd);
            TCLAP::SwitchArg resetSwitch(
                "r", "reset", "Reset profiling data after showing them", cmd, false);
            cmd.parse(argc-1, argv+1);

            Request::Runtime *profiling = req.mutable_runtime();

            if(nameArg.isSet()) {
                profiling->set_name(nameArg.getValue());
            }
            profiling->mutable_profiling()->set_reset(resetSwitch.getValue());
            socket.writeMessage(req);

            Response res;
            socket.readMessage(res);
            expectResponseType(res, Response::ContentCase::kProfilingSummary);
            const auto &profSumm = res.profiling_summary();
            size_t maxNameLen = 0;
            for(int i = 0; i < profSumm.traces().size(); i++) {
                maxNameLen = std::max(maxNameLen, profSumm.traces(i).name().length());
            }
            for(int i = 0; i < profSumm.traces_size(); i++) {
                const auto &trace = profSumm.traces(i);
                if(trace.has_running_since()) {
                    std::cout << lms::internal::COLOR_GREEN;
                }
                std::cout << trace.name();
                size_t numPadSpaces = maxNameLen - trace.name().length();
                while(numPadSpaces -- > 0) {
                    std::cout << " ";
                }
                std::cout << " #" << trace.count()
                          << "\t\u00F8 " << trace.avg()
                          << "\t\u00B1 " << trace.std()
                          << "\t [" << trace.min() << "; " << trace.max() << "]";
                if(trace.has_running_since()) {
                    std::cout << "\t\u2192 " << trace.running_since();
                    std::cout << lms::internal::COLOR_WHITE;
                }
                std::cout << std::endl;

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
        std::cout << "  attach <name> - Attach to running runtime\n";
        std::cout << "  kill <name> - Kill runtime (SIGKILL)\n";
        std::cout << "  stop <name> - Stop runtime (SIGINT)\n";
        std::cout << "  profiling <name> - Show profiling\n";
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
