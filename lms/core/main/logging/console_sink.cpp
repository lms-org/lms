#include <core/logger.h>
#include <core/extra/colors.h>
#include <ctime>

namespace lms {
namespace logging {

ConsoleSink& ConsoleSink::printTime(bool time) {
    m_time = time;
    return *this;
}

ConsoleSink& ConsoleSink::printColored(bool colored) {
    m_colored = colored;
    return *this;
}

void ConsoleSink::sink(const LogMessage &message) {
    if(m_time) {
        // get time now
        time_t t = ::time(0);
        struct tm * now = localtime( & t );

        // format time
        if(now->tm_hour < 10) {
            m_out << "0";
        }
        m_out << now->tm_hour << ":";
        if(now->tm_min < 10) {
            m_out << "0";
        }
        m_out << now->tm_min << ":";
        if(now->tm_sec < 10) {
            m_out << "0";
        }
        m_out << now->tm_sec << " ";
    }
    if(m_colored) {
        m_out << Logger::levelColor(message.level);
    }
    m_out << Logger::levelName(message.level) << " " << message.tag;
    if(m_colored) {
        m_out << COLOR_WHITE;
    }
    m_out << " " << message.messageText() << std::endl;
}

} // namespace logging
} // namespace lms

