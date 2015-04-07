#include <memory>
#include <utility>

#include <lms/logger.h>

namespace lms {
namespace logging {

ChildLogger::ChildLogger(const std::string &name, Logger *parent,
                         std::unique_ptr<LoggingFilter> filter)
    : parent(parent), name(name), m_filter(std::move(filter)) {
    std::cout << "New child logger "<< name << std::endl;
}

std::unique_ptr<LogMessage> ChildLogger::log(LogLevel lvl, const std::string& tag) {
    if(!m_filter || m_filter->filter(lvl, tag)) {
        if(parent == nullptr) {
            std::cerr << "CHILD LOGGER " << name << " IS NOT INITIALIZED." << std::endl;
            return nullptr;
        }

        if(tag.empty()) {
            return parent->log(lvl, name);
        } else {
            std::ostringstream newTag;
            newTag << name << "." << tag;
            return parent->log(lvl, newTag.str());
        }
    } else {
        return nullptr;
    }
}

void ChildLogger::filter(std::unique_ptr<LoggingFilter> filter) {
    m_filter = std::move(filter);
}

} // namespace logging
} // namespace lms

