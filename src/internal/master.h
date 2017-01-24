#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <list>
#include <cmath>

#include "lms/protobuf_socket.h"
#include "messages.pb.h"
#include "lms/logger.h"

namespace lms {
namespace internal {

class MasterServer {
public:
    MasterServer();
    int useUnix(const std::string &path);
    int useIPv4(int port);
    void start();

private:
    struct Client {
        Client(int fd, const std::string &peer);
        ProtobufSocket sock;
        std::string peer;
        bool isAttached;
        pid_t attachedRuntime;
        bool shutdownRuntimeOnDetach;
        bool listenBroadcats = false;
        logging::Level logLevel = logging::Level::ALL;
    };

    struct Server {
        Server(int fd);
        int fd;
    };

    struct Runtime {
        std::string name;
        pid_t pid;
        ProtobufSocket logSock;
        ProtobufSocket commSock;
        std::string config_file;
    };

    std::vector<Server> m_servers;
    std::vector<Client> m_clients;
    std::vector<Runtime> m_runtimes;
    bool m_running;
    int runtimeNameCounter = 1;

    void enableNonBlock(int sock);
    void processClient(Client &client, const lms::Request &message);
    void runFramework(Client &client, const Request_Run &options);
    void broadcastResponse(const lms::Response &response);
    void buildListRuntimesResponse(lms::Response &response);
    void buildListClientsResponse(lms::Response &response);
    Runtime* getRuntimeByName(const std::string &name);
    Runtime* getRuntimeByPid(pid_t id);
    bool isSomeoneAttached(pid_t id);
};

void setLogLevel(ProtobufSocket &sock, logging::Level level);

void connectToMaster(int argc, char *argv[]);
}
}
