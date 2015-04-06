#include <memory>

#include <lms/logger.h>

namespace lms {
namespace logging {

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

} // namespace logging
} // namespace lms

