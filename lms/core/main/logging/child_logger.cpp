#include <memory>

#include <core/logger.h>

namespace lms {
namespace logging {

std::unique_ptr<LogMessage> ChildLogger::log(LogLevel lvl, const std::string& tag) {
    if(parent == nullptr) {
        std::cerr << "CHILD LOGGER " << name << " IS NOT INITIALIZED." << std::endl;
    }

    if(tag.empty()) {
        return parent->log(lvl, name);
    } else {
        std::ostringstream newTag;
        newTag << name << "." << tag;
        return parent->log(lvl, newTag.str());
    }
}

} // namespace logging
} // namespace lms

