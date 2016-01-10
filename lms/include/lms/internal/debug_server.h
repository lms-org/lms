#ifndef LMS_INTERNAL_DEBUG_SERVER_H
#define LMS_INTERNAL_DEBUG_SERVER_H

#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "lms/logger.h"

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

    bool useUnix(std::string const& path);

    bool useIPv4(uint16_t port);

    bool useIPv6(uint16_t port);

    bool useDualstack(uint16_t port);

    void process();

    void startThread();

    void broadcast(std::uint8_t type, Datagram const& datagram);
private:
    void enableReuseAddr(int sock);

    void enableIPv6Only(int sock);

    void startListening(int sock);

    void enableNonBlock(int sock);

    struct Client {
        Client() : valid(false), buffer(4096), bufferUsed(0) {}

        int sockfd;
        bool valid;
        std::vector<std::uint8_t> buffer;
        size_t bufferUsed;
    };

    logging::Logger logger;

    std::vector<int> m_server;
    fd_set m_rfds;
    std::vector<Client> m_clients;
    std::thread m_thread;
    bool m_shutdown;

    void processServer(int sockfd);
    void processClient(Client & client);
};

}  // namespace internal
}  // namespace lms

#endif // LMS_INTERNAL_DEBUG_SERVER_H
