#include <memory>
#include <utility>

#include <lms/logger.h>

namespace lms {
namespace logging {

ChildLogger::ChildLogger(Context *ctx, const std::string &name, Level threshold)
    : m_context(ctx), m_name(name), m_threshold(threshold) {}

ChildLogger::ChildLogger(const std::string &name, Level threshold)
    : m_context(& Context::getDefault()), m_name(name), m_threshold(threshold) {}

std::unique_ptr<LogMessage> ChildLogger::log(Level lvl, const std::string& tag) {
    if(m_context == nullptr) {
        std::cerr << "LOGGER " << m_name << " HAS NO VALID CONTEXT" << std::endl;
        return nullptr;
    }

    Filter *filter = m_context->filter();

    std::string newTag;

    if(tag.empty()) {
        // if no tag was given, just use the logger's name
        newTag = m_name;
    } else {
        // otherwise concatenate with the given tag
        newTag = m_name + "." + tag;
    }

    if(lvl >= m_threshold && (filter == nullptr || filter->decide(lvl, newTag))) {
        return std::unique_ptr<LogMessage>(new LogMessage(*m_context, lvl, newTag));
    } else {
        return nullptr;
    }
}

std::string ChildLogger::name() const {
    return m_name;
}

void ChildLogger::name(const std::string &name) {
    m_name = name;
}

Level ChildLogger::threshold() const {
    return m_threshold;
}

void ChildLogger::threshold(Level level) {
    m_threshold = level;
}

} // namespace logging
} // namespace lms

