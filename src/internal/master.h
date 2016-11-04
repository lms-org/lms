#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <list>
#include <cmath>

#include "lms/protobuf_socket.h"
#include "messages.pb.h"
#include "../time.h"
#include "lms/logger.h"

namespace lms {
namespace internal {

template<typename T>
class Trace {
public:
    Trace() : m_avg(0), m_squaredAvg(0),
        m_min(std::numeric_limits<T>::max()), m_max(0), m_count(0) {}
    void update(T x) {
        m_count ++; // increment first
        // http://stackoverflow.com/a/1934266
        m_avg += (x - m_avg) / m_count;
        m_squaredAvg += (x * x - m_squaredAvg) / m_count;
        if(x > m_max) {
            m_max = x;
        }
        if(x < m_min) {
            m_min = x;
        }
    }
    T avg() const { return m_avg; }
    T min() const { return m_min; }
    T max() const { return m_max; }
    T var() const { return m_squaredAvg - m_avg * m_avg; }
    T std() const { return std::sqrt(var()); }
    unsigned int count() const { return m_count; }
    void reset() {
        m_avg = 0;
        m_squaredAvg = 0;
        m_min = std::numeric_limits<T>::max();
        m_max = 0;
        m_count = 0;
    }
private:
    T m_avg;
    T m_squaredAvg;
    T m_min;
    T m_max;
    unsigned int m_count;
};

class Profiler {
public:
    void addMeasurement(const Response::LogEvent &event);
    void getOverview(Response::ProfilingSummary *summary) const;
private:
    std::map<std::string, lms::Time> beginTimes;
    std::map<std::string, Trace<float>> measurements;
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
        bool listenBroadcats = false;
        logging::Level logLevel = logging::Level::ALL;
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

    void enableNonBlock(int sock);
    void processClient(Client &client, const lms::Request &message);
    void runFramework(Client &client, const Request_Run &options);
    void broadcastResponse(const lms::Response &response);
    void buildListRuntimesResponse(lms::Response &response);
    void buildListClientsResponse(lms::Response &response);
};

void connectToMaster(int argc, char *argv[]);
}
}
