#include <lms/logger.h>
#include <lms/internal/colors.h>
#include <ctime>

namespace lms {
namespace logging {

void ConsoleSink::printTime(bool time) { m_time = time; }

void ConsoleSink::printColored(bool colored) { m_colored = colored; }

void ConsoleSink::sink(const Event &message) {
    std::lock_guard<std::mutex> lck(mtx);

    if (m_time) {
        // get time now
        time_t rawtime;
        std::time(&rawtime);
        struct tm *now = std::localtime(&rawtime);

        // format time to "HH:MM:SS"
        char buffer[10];
        std::strftime(buffer, 10, "%T", now);

        m_out << buffer << " ";
    }
    if (m_colored) {
        m_out << levelColor(message.level);
    }
    m_out << levelName(message.level) << " " << message.tag;
    if (m_colored) {
        m_out << lms::internal::COLOR_WHITE;
    }
    m_out << " " << message.messageText() << std::endl;
}

} // namespace logging
} // namespace lms
