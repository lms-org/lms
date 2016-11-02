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

void Profiler::addMeasurement(const Response::LogEvent &event) {
    if(event.level() == Response::LogEvent::PROFILE) {
        if(event.text() == "_begin") {
            // memorize new begin timestamp
            beginTimes[event.tag()] = lms::Time::fromMicros(event.timestamp());
        } else if(event.text() == "_end") {
            auto it = beginTimes.find(event.tag());
            if(it == beginTimes.end()) {
                // could not find begin time
            } else {
                lms::Time begin = it->second;
                lms::Time end = lms::Time::fromMicros(event.timestamp());
                auto &times = measurements[event.tag()];
                times.push_back(end - begin);
                if(times.size() > 100) {
                    // hold only a limited number of measurements
                    times.pop_front();
                }
            }
        }
    }
}

void Profiler::getOverview(Response::ProfilingSummary *summary) const {
    for(const auto &pair : measurements) {
        if(pair.second.size() == 0) {
            continue;
        }
        float sum_x = 0;
        float sum_x_squared = 0;
        std::int64_t max = 0;
        std::int64_t min = std::numeric_limits<decltype(min)>::max();
        for(auto time : pair.second) {
            sum_x += time.micros();
            sum_x_squared += time.micros() * time.micros();
            if(time.micros() > max) {
                max = time.micros();
            }
            if(time.micros() < min) {
                min = time.micros();
            }
        }
        float avg = sum_x / float(pair.second.size());
        float var = sum_x_squared / pair.second.size() - avg * avg;
        // stanard deviation is square root of variance
        float std = std::sqrt(var);

        Response::ProfilingSummary::Trace *trace = summary->add_traces();
        trace->set_name(pair.first);
        trace->set_count(pair.second.size());
        trace->set_avg(avg);
        trace->set_min(min);
        trace->set_max(max);
        trace->set_std(std);
    }
}

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
                std::cout << "New Client" << std::endl;
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
                std::cout << "Client packet" << std::endl;
                bool exit = false;

                lms::Request req;
                auto readRes = client.sock.readMessage(req);

                if(readRes == ProtobufSocket::OK) {
                    processClient(client, req);
                } else {
                    if(client.isAttached && client.shutdownRuntimeOnDetach) {
                        kill(client.attachedRuntime, SIGINT);
                    }
                    client.sock.close();
                    m_clients.erase(it);
                    --it;
                }
            }
        }
        for(auto it = m_runtimes.begin(); it != m_runtimes.end(); ++it) {
            auto &runtime = *it;
            if(FD_ISSET(runtime.sock.getFD(), &fds)) {
                // forward log events to attached clients
                Response response;
                if(runtime.sock.readMessage(response) == ProtobufSocket::OK) {
                    if(response.has_log_event() &&
                            response.log_event().level() == Response::LogEvent::PROFILE) {
                        runtime.profiler.addMeasurement(response.log_event());
                    }

                    for(auto &client : m_clients) {
                        if(client.isAttached && client.attachedRuntime == runtime.pid) {
                            client.sock.writeMessage(response);
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

                    // close runtime connection
                    runtime.sock.close();
                    m_runtimes.erase(it);
                    it--;
                    std::cout << "Could not read msg from runtime" << std::endl;
                    //TODO error handling
                }
            }
        }
    }
}

void MasterServer::processClient(Client &client, const lms::Request &message) {
    Response response;

    switch(message.content_case()) {
    case lms::Request::kInfo:
        {
        auto info = response.mutable_info();
        info->set_version(LMS_VERSION_CODE);
        info->set_pid(getpid());
        }
        break;
    case lms::Request::kListClients:
        {
        auto list = response.mutable_client_list();
        for (const auto &cl : m_clients) {
            auto client = list->add_clients();
            client->set_fd(cl.sock.getFD());
            client->set_peer(cl.peer);
        }
        }
        break;
    case lms::Request::kListProcesses:
        {
        auto list = response.mutable_process_list();
        for(const auto &rt : m_runtimes) {
            auto *process = list->add_processes();
            process->set_pid(rt.pid);
            process->set_config_file(rt.config_file);
        }
        }
        break;
    case lms::Request::kShutdown:
        m_running = false;
        break;
    case lms::Request::kRun:
        runFramework(client, message.run());
        break;
    case lms::Request::kAttach:
        client.isAttached = true;
        client.attachedRuntime = atoi(message.attach().id().c_str());
        client.shutdownRuntimeOnDetach = false;
        break;
    case lms::Request::kStop:
        {
        int signal = message.stop().kill() ? SIGKILL : SIGINT;
        kill(atoi(message.stop().id().c_str()), signal);
        }
        break;
    case lms::Request::kProfiling:
        {
        int requestedId = atoi(message.profiling().id().c_str());
        for(const auto &rt : m_runtimes) {
            if(rt.pid == requestedId) {
                rt.profiler.getOverview(response.mutable_profiling_summary());
                break;
            }
        }
        // TODO runtime not found
        }
        break;
    }

    client.sock.writeMessage(response);

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
        //Write to file
        //std::ofstream fs("/tmp/gangster.txt");
        //ctx.appendSink(new logging::ConsoleSink(fs));

        //logging::Logger logg("MyLogger");
        //logg.info("test") << fd[0] << " " << fd[1];

        lms::internal::Framework fw(options.config_file());
        fw.setDebug(options.debug());
        for(int i = 0; i < options.include_paths_size(); i++) {
            fw.addSearchPath(options.include_paths(i));
        }
        for(int i = 0; i < options.flags_size(); i++) {
            fw.addFlag(options.flags(i));
        }

        SignalHandler::getInstance().addListener(SIGSEGV, &fw);
        SignalHandler::getInstance().addListener(SIGINT, &fw);

        fw.start();
        exit(0);
    } else {
        // master server

        // close writing ends of pipes
        close(fd[1]);

        Runtime rt{childpid, fd[0], options.config_file()};
        if(! options.detached()) {
            client.isAttached = true;
            client.attachedRuntime = childpid;
            if(options.shutdown_runtime_on_detach()) {
                client.shutdownRuntimeOnDetach = true;
            }
        }
        m_runtimes.push_back(rt);
    }
}

void streamLogs(ProtobufSocket &socket, logging::Level logLevel) {
    Response response;
    while(socket.readMessage(response) == ProtobufSocket::OK) {
        if(!response.has_log_event()) continue;

        const auto &event = response.log_event();
        if(static_cast<logging::Level>(event.level()) >= logLevel) {
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
        }

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
            std::cout << "ID \tCONFIG\n";
            for(int i = 0; i < list.processes_size(); i++) {
                std::cout << list.processes(i).pid() << " \t" << list.processes(i).config_file() << "\n";
            }
        } else if(strcmp(argv[1], "shutdown") == 0) {
            req.mutable_shutdown();
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "run") == 0) {
            std::vector<std::string> logLevels = {"all",  "debug", "info",
                                                    "warn", "error"};
            TCLAP::ValuesConstraint<std::string> logConstraint(logLevels);

            TCLAP::CmdLine cmd("lms run", ' ', LMS_VERSION_STRING);
            TCLAP::UnlabeledValueArg<std::string> configArg(
                "config", "XML config path", true, "lms.xml", "XML Config", cmd);
            TCLAP::MultiArg<std::string> loadPathsArg(
                "p", "path", "Add additional load path", false, "Path", cmd);
            TCLAP::MultiArg<std::string> flagsArg(
                "f", "flag", "Add flag for XML config", false, "Flag", cmd);
            TCLAP::SwitchArg debugSwitch(
                "", "debug", "Make a ridiculous number of debug outputs", cmd, false);
            TCLAP::ValueArg<std::string> logArg(
                "", "log", "Minimum logging level",
                false, "ALL", &logConstraint, cmd);
            TCLAP::SwitchArg detachSwitch(
                "d", "detach", "Start runtime but do not show logging messages", cmd, false);
            TCLAP::SwitchArg shutdownOnDetachSwitch(
                "s", "shutdown-on-detach", "Shutdown runtime when clients disconnects", cmd, false);
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

            char *lms_path = std::getenv("LMS_PATH");
            if (lms_path != nullptr && lms_path[0] != '\0') {
                for (auto const &path : split(lms_path, ':')) {
                    *run->add_include_paths() = path;
                }
            }

            socket.writeMessage(req);

            logging::Level logLevel = logging::Level::ALL;
            logging::levelFromName(logArg.getValue(), logLevel);

            if(! detachSwitch.getValue()) {
                streamLogs(socket, logLevel);
            }
        } else if(strcmp(argv[1], "attach") == 0) {
            lms::Request_Attach *attach = req.mutable_attach();

            if(argc >= 3) {
                attach->set_id(argv[2]);
                socket.writeMessage(req);
            } else {
                std::cout << "Requires argument: lms attach <id> \n";
            }

            streamLogs(socket, logging::Level::ALL);
        } else if(strcmp(argv[1], "kill") == 0 || strcmp(argv[1], "stop") == 0) {
            lms::Request_Stop *stop = req.mutable_stop();

            if(argc >= 3) {
                stop->set_id(argv[2]);
                stop->set_kill(strcmp(argv[1], "kill") == 0);
                socket.writeMessage(req);
            } else {
                std::cout << "Requires argument: lms kill <id>\n";
            }
        } else if(strcmp(argv[1], "modules") == 0) {
            lms::Request_ModuleList *modules = req.mutable_module_list();
            socket.writeMessage(req);
        } else if(strcmp(argv[1], "profiling") == 0) {
            Request::Profiling *profiling = req.mutable_profiling();

            if(argc >= 3) {
                profiling->set_id(argv[2]);
                socket.writeMessage(req);
                socket.writeMessage(req);

                Response res;
                socket.readMessage(res);
                expectResponseType(res, Response::ContentCase::kProfilingSummary);
                const auto &profiling = res.profiling_summary();
                for(int i = 0; i < profiling.traces_size(); i++) {
                    const auto &trace = profiling.traces(i);
                    std::cout << trace.name() << "\t#" << trace.count()
                              << "\t\u00F8 " << trace.avg()
                              << "\t\u00B1 " << trace.std()
                              << "\t [" << trace.min() << "; " << trace.max() << "]"
                              << std::endl;
                }
            } else {
                std::cout << "Requires argument: lms profiling <id> \n";
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
