#ifndef LMS_INTERNAL_DEBUG_SERVER_H
#define LMS_INTERNAL_DEBUG_SERVER_H

#include <string>
#include <vector>
#include <memory>
#include <thread>

namespace lms {
namespace internal {

class DebugServer {
public:
    DebugServer();
    ~DebugServer();

    struct Datagram {
        std::vector<std::uint8_t> data;
    };

    enum class MessageType : std::uint8_t {
        LOGGING = 1, PROFILING = 2
    };

    bool useUnixSocket(std::string const& path);

    void process();

    void startThread();

    void broadcast(std::uint8_t type, Datagram const& datagram);
private:
    struct Client {
        Client() : valid(false), buffer(4096), bufferUsed(0) {}

        int sockfd;
        bool valid;
        std::vector<std::uint8_t> buffer;
        size_t bufferUsed;
    };

    int m_sockfd;
    fd_set m_rfds;
    std::vector<Client> m_clients;
    std::thread m_thread;
    bool m_shutdown;

    void acceptClients();
    void processClient(Client & client);
};

}  // namespace internal
}  // namespace lms

#endif // LMS_INTERNAL_DEBUG_SERVER_H
