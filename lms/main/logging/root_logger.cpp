#include <memory>

#include <lms/logger.h>

namespace lms {
namespace logging {

Context& Context::getDefault() {
    static Context ctx;
    return ctx;
}

Context::Context() {
}

void Context::appendSink(Sink *sink) {
    m_sinks.push_back(std::move(std::unique_ptr<Sink>(sink)));
}

void Context::clearSinks() {
    m_sinks.clear();
}

bool Context::hasFilter() {
    return static_cast<bool>(m_filter);
}

void Context::filter(Filter *filter) {
    m_filter.reset(filter);
}

Filter* Context::filter() const {
    return m_filter.get();
}

void Context::processMessage(const LogMessage &message) {
    for(size_t i = 0; i < m_sinks.size(); i++) {
        m_sinks[i]->sink(message);
    }
}

} // namespace logging
} // namespace lms

