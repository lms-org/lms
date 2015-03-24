#include <lms/logger.h>
#include <lms/extra/colors.h>
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
        time_t rawtime;
        std::time(&rawtime);
        struct tm *now = std::localtime(&rawtime);

        // format time to "HH:MM:SS"
        char buffer[10];
        std::strftime(buffer, 10, "%T", now);

        m_out << buffer << " ";
    }
    if(m_colored) {
        m_out << levelColor(message.level);
    }
    m_out << levelName(message.level) << " " << message.tag;
    if(m_colored) {
        m_out << COLOR_WHITE;
    }
    m_out << " " << message.messageText() << std::endl;
}

} // namespace logging
} // namespace lms

