#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <list>

#include "lms/protobuf_socket.h"
#include "messages.pb.h"
#include "../time.h"

namespace lms {
namespace internal {

class Profiler {
public:
    void addMeasurement(const Response::LogEvent &event);
    void getOverview(Response::ProfilingSummary *summary) const;
private:
    std::map<std::string, lms::Time> beginTimes;
    std::map<std::string, std::list<lms::Time>> measurements;
};

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
    };

    struct Server {
        Server(int fd);
        int fd;
    };

    struct Runtime {
        pid_t pid;
        ProtobufSocket sock;
        std::string config_file;
        Profiler profiler;
    };

    std::vector<Server> m_servers;
    std::vector<Client> m_clients;
    std::vector<Runtime> m_runtimes;
    bool m_running;

    enum class ClientResult { exit, attached };

    void enableNonBlock(int sock);
    ClientResult processClient(Client &client, const lms::Request &message);
    void runFramework(Client &client, const Request_Run &options);
};

void connectToMaster(int argc, char *argv[]);
}
}
