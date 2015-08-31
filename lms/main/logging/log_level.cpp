#include "lms/logging/log_level.h"
#include "lms/extra/colors.h"

namespace lms {
namespace logging {

std::string levelName(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::ALL : return "ALL";
    case LogLevel::DEBUG : return "DEBUG";
    case LogLevel::INFO :  return "INFO ";
    case LogLevel::WARN :  return "WARN ";
    case LogLevel::ERROR : return "ERROR";
    case LogLevel::OFF : return "OFF";
    }
    return "?";
}

bool levelFromName(const std::string &name, LogLevel &level) {
    if(name == "ALL") level = LogLevel::ALL;
    else if(name == "DEBUG") level = LogLevel::DEBUG;
    else if(name == "INFO") level = LogLevel::INFO;
    else if(name == "WARN") level = LogLevel::WARN;
    else if(name == "ERROR") level = LogLevel::ERROR;
    else if(name == "OFF") level = LogLevel::OFF;
    else return false;

    return true;
}

std::string levelColor(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return lms::extra::COLOR_GREEN;
    case LogLevel::INFO : return lms::extra::COLOR_BLUE;
    case LogLevel::WARN : return lms::extra::COLOR_YELLOW;
    case LogLevel::ERROR : return lms::extra::COLOR_RED;
    default: return lms::extra::COLOR_WHITE; // ALL, OFF
    }
}

}  // namespace logging
}  // namespace lms
