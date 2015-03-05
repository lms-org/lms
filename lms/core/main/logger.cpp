#include <core/logger.h>

namespace lms{
LogMessage::~LogMessage() {
    sink->sink(*this);
}

std::unique_ptr<LogMessage> Logger::debug(const std::string& tag) {
    return log(LogLevel::DEBUG, tag);
}

std::unique_ptr<LogMessage> Logger::info(const std::string& tag) {
    return log(LogLevel::INFO, tag);
}

std::unique_ptr<LogMessage> Logger::warn(const std::string& tag) {
    return log(LogLevel::WARN, tag);
}

std::unique_ptr<LogMessage> Logger::error(const std::string& tag) {
    return log(LogLevel::ERROR, tag);
}

std::string Logger::levelName(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return "DEBUG";
    case LogLevel::INFO :  return "INFO ";
    case LogLevel::WARN :  return "WARN ";
    case LogLevel::ERROR : return "ERROR";
    default : return "_"; // this should never happen
    }
}

std::string Logger::levelColor(LogLevel lvl) {
    switch(lvl) {
    case LogLevel::DEBUG : return COLOR_WHITE;
    case LogLevel::INFO : return COLOR_BLUE;
    case LogLevel::WARN : return COLOR_YELLOW;
    case LogLevel::ERROR : return COLOR_RED;
    default: return COLOR_WHITE; // this should never happen
    }
}

RootLogger::RootLogger(std::unique_ptr<Sink> sink) {
    std::cout << "New root logger with sink" << std::endl;
    m_sink = std::move(sink);
}

RootLogger::RootLogger() {
    std::cout << "New root logger" << std::endl;
    m_sink.reset(new ConsoleSink());
}

void RootLogger::sink(std::unique_ptr<Sink> sink) {
    m_sink = std::move(sink);
}

std::unique_ptr<LogMessage> RootLogger::log(LogLevel lvl, const std::string& tag) {
    return std::unique_ptr<LogMessage>(new LogMessage(m_sink.get(), lvl, tag));
}

std::unique_ptr<LogMessage> ChildLogger::log(LogLevel lvl, const std::string& tag) {
    if(tag.empty()) {
        return parent->log(lvl, name);
    } else {
        std::ostringstream newTag;
        newTag << name << "." << tag;
        return parent->log(lvl, newTag.str());
    }
}

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

std::unique_ptr<LogMessage> operator <<(std::unique_ptr<LogMessage> message, std::ostream& (*pf) (std::ostream&))
{
    if(message.get() == nullptr) {
        std::cerr << "LOG MESSAGE IS NULL" << std::endl;
    } else {
        // check if someone tried to write std::endl
        // -> that will end the log message and we can write it to a sink
        if(pf == (std::ostream& (*) (std::ostream&))std::endl) {
            // first thing we do here:
            // - destruct the LogMessage
            // -> that will trigger the flush method
            // -> we write the log message
            // second thing we do here:
            // - create a new log message with the same parameters as the current message
            // - put it into the unique pointer
            // - return the new wrapped log message
            message.reset(new LogMessage(message->sink, message->level, message->tag));
        } else {
            // in case of any other stream modifier (e.g. std::hex)
            // -> just write it into the stringstream
            message->messageStream << pf;
        }
    }
    return message;
}
}
