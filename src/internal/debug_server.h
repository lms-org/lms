#ifndef LMS_INTERNAL_DEBUG_SERVER_H
#define LMS_INTERNAL_DEBUG_SERVER_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <queue>

#include "lms/logger.h"

namespace lms {
namespace internal {

class DebugServer {
public:
    DebugServer();
    ~DebugServer();

    enum class MessageType : std::uint8_t { LOGGING = 1, PROFILING = 2 };

    class Datagram {
    public:
        Datagram(MessageType type, uint32_t messageLen);
        uint8_t *data();

        const uint8_t *internal() const;
        size_t size() const;

    private:
        // 32 bit length + 8 bit message type
        static constexpr size_t HEADER_LEN = sizeof(uint32_t) + sizeof(uint8_t);
        std::vector<uint8_t> m_data;
    };

    bool useUnix(std::string const &path);

    bool useIPv4(uint16_t port);

    bool useIPv6(uint16_t port);

    bool useDualstack(uint16_t port);

    void startThread();

    void broadcast(Datagram const &datagram);

private:
    void enableReuseAddr(int sock);

    void enableIPv6Only(int sock);

    void startListening(int sock);

    void enableNonBlock(int sock);

    struct Client {
        Client() : valid(false), buffer(4096), bufferUsed(0), outOffset(0) {}

        int sockfd;
        bool valid;
        std::vector<std::uint8_t> buffer;
        size_t bufferUsed;

        std::queue<Datagram> outBuffer;
        size_t outOffset;
    };

    logging::Logger logger;

    std::vector<int> m_server;
    fd_set m_readRfds;
    fd_set m_writeRfds;
    std::vector<Client> m_clients;
    std::thread m_thread;
    bool m_shutdown;

    std::mutex m_outMutex;

    void processWrites();
    void processReads();
    void processServer(int sockfd);
    void processClient(Client &client);
    void processOutqueue(Client &client);
};

} // namespace internal
} // namespace lms

#endif // LMS_INTERNAL_DEBUG_SERVER_H
