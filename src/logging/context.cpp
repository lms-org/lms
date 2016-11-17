#include <memory>

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

} // namespace logging
} // namespace lms
