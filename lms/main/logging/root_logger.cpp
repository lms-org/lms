#include <memory>

#include <lms/logger.h>

namespace lms {
namespace logging {

RootLogger::RootLogger(std::unique_ptr<Sink> sink, std::unique_ptr<LoggingFilter> filter) :
    m_sink(std::move(sink)), m_filter(std::move(filter)) {
}

RootLogger::RootLogger() : m_sink(new ConsoleSink()), m_filter(nullptr) {
}

void RootLogger::sink(std::unique_ptr<Sink> sink) {
    m_sink = std::move(sink);
}

void RootLogger::filter(std::unique_ptr<LoggingFilter> filter) {
    m_filter = std::move(filter);
}

std::unique_ptr<LogMessage> RootLogger::log(LogLevel lvl, const std::string& tag) {
    if(!m_filter || m_filter->filter(lvl, tag)) {
        return std::unique_ptr<LogMessage>(new LogMessage(*m_sink, lvl, tag));
    } else {
        return nullptr;
    }
}

} // namespace logging
} // namespace lms

