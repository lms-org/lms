#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <thread>

#include <lms/logger.h>
#include "../internal/profiler.h"

namespace lms {
namespace logging {

struct Context::Private {
    std::vector<std::unique_ptr<Sink>> m_sinks;
    std::unique_ptr<Filter> m_filter;
    internal::Profiler profiler;
    std::mutex profilerMutex;
    std::mutex loggingMutex;
    logging::Level level = logging::Level::ALL;
    std::thread hookThread;
};

Context &Context::getDefault() {
    static Context ctx;
    return ctx;
}

Context::Context() : dptr(new Private) {}

Context::~Context() {
    delete dptr;
}

void Context::appendSink(Sink *sink) {
    std::lock_guard<std::mutex> lock(dfunc()->loggingMutex);
    dfunc()->m_sinks.push_back(std::unique_ptr<Sink>(sink));
}

void Context::clearSinks() { dfunc()->m_sinks.clear(); }

bool Context::hasFilter() { return static_cast<bool>(dfunc()->m_filter); }

void Context::filter(Filter *filter) { dfunc()->m_filter.reset(filter); }

Filter *Context::filter() const { return dfunc()->m_filter.get(); }

void Context::processMessage(const Event &message) {
    std::lock_guard<std::mutex> lock(dfunc()->loggingMutex);
    if(message.level >= dfunc()->level) {
        for (size_t i = 0; i < dfunc()->m_sinks.size(); i++) {
            dfunc()->m_sinks[i]->sink(message);
        }
    }
}

void Context::resetProfiling() {
    std::lock_guard<std::mutex> lock(dfunc()->profilerMutex);
    dfunc()->profiler.reset();
}

void Context::time(const std::string &tag) {
    std::lock_guard<std::mutex> lock(dfunc()->profilerMutex);
    dfunc()->profiler.addBegin(tag, lms::Time::now());
}

void Context::timeEnd(const std::string &tag) {
    std::lock_guard<std::mutex> lock(dfunc()->profilerMutex);
    dfunc()->profiler.addEnd(tag, lms::Time::now());
}

void Context::profilingSummary(std::map<std::string, Trace<double>> &measurements) {
    std::lock_guard<std::mutex> lock(dfunc()->profilerMutex);
    dfunc()->profiler.getOverview(measurements);
}

void Context::setLevel(logging::Level level) {
    std::lock_guard<std::mutex> lock(dfunc()->loggingMutex);
    dfunc()->level = level;
}

void processFD(int fd, char* buf, size_t bufSize, Context *ctx, const char* tag, Level lvl) {
    size_t readBytes = read(fd, buf, bufSize);
    if(readBytes > 0) {
        Event evt(*ctx, lvl, tag, lms::Time::now());
        std::string msg(buf, readBytes);
        if(msg.size() > 0 && msg[msg.size()-1] == '\n') {
            msg = msg.substr(0, msg.size()-1);
        }
        evt.messageStream << msg;
    }
}

void Context::hookStdoutAndStderr() {
    // create pipes
    int out[2];
    int err[2];
    ::pipe(out);
    ::pipe(err);

    // close old stdout and stderr (ignoring any errors)
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // replace stdout and stderr by our pipes
    dup2(out[1], STDOUT_FILENO);
    dup2(err[1], STDERR_FILENO);

    // now start the thread that does all the magic
    dfunc()->hookThread = std::thread([this, out, err] () {
        fd_set fds;
        constexpr int BUF_SIZE = 1024;
        char buf[BUF_SIZE];

        while(true) {
            FD_ZERO(&fds);
            FD_SET(out[0], &fds);
            FD_SET(err[0], &fds);

            timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int ret = select(std::max(out[0], err[0]) + 1, &fds, nullptr, nullptr, &timeout);

            if (ret == -1) {
                break;
            }

            if(FD_ISSET(out[0], &fds)) {
                processFD(out[0], buf, BUF_SIZE, this, "stdout", Level::INFO);
            }
            if(FD_ISSET(err[0], &fds)) {
                processFD(err[0], buf, BUF_SIZE, this, "stderr", Level::ERROR);
            }
        }
    });
    dfunc()->hookThread.detach();
}

} // namespace logging
} // namespace lms
