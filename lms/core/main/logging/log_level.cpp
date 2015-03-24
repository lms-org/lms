#include "lms/logging/log_level.h"
#include "lms/extra/colors.h"

namespace lms {
namespace logging {

std::string levelName(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return "DEBUG";
    case LogLevel::INFO :  return "INFO ";
    case LogLevel::WARN :  return "WARN ";
    case LogLevel::ERROR : return "ERROR";
    default : return "_"; // this should never happen
    }
}

LogLevel levelFromName(const std::string &name) {
    if(name == "DEBUG") return LogLevel::DEBUG;
    if(name == "INFO") return LogLevel::INFO;
    if(name == "WARN") return LogLevel::WARN;
    if(name == "ERROR") return LogLevel::ERROR;
    return SMALLEST_LEVEL;
}

std::string levelColor(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return COLOR_GREEN;
    case LogLevel::INFO : return COLOR_BLUE;
    case LogLevel::WARN : return COLOR_YELLOW;
    case LogLevel::ERROR : return COLOR_RED;
    default: return COLOR_WHITE; // this should never happen
    }
}

}  // namespace logging
}  // namespace lms
