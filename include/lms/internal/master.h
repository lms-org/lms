#pragma once

#include <string>
#include <vector>

#include "lms/internal/line_separated_reader.h"

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
        int fd;
        LineReader reader;
        LineWriter writer;
        std::string peer;
    };

    struct Server {
        Server(int fd);
        int fd;
    };

    std::vector<Server> m_servers;
    std::vector<Client> m_clients;
    std::vector<int> m_runtimes;
    bool m_running;

    void enableNonBlock(int sock);
    void processClient(Client &client, const std::string &message);
};

class MasterClient {
public:
    static MasterClient fromUnix(const std::string &path);
    ~MasterClient();
    int fd() const;
private:
    int m_sockfd;
    MasterClient(int fd);
};

void connectToMaster(int argc, char *argv[]);

}
}
