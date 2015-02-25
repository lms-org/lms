#ifndef LMS_CORE_LOGGER_H
#define LMS_CORE_LOGGER_H

#include <iostream>
#include <cstdint>
#include <sstream>
#include <ctime>
#include <memory>

#include <core/extra/colors.h>

class LogMessage;

class Sink {
public:
    virtual ~Sink() {}
    virtual void sink(const LogMessage &message) = 0;
};

enum class LogLevel : std::int8_t {
    DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4
};

class LogMessage {
public:
    LogMessage(Sink *sink, LogLevel lvl, const std::string& tag)
        : m_sink(sink), m_lvl(lvl), m_tag(tag) {}
    ~LogMessage() {
        flush();
    }

    std::string tag() const {
        return m_tag;
    }

    LogLevel level() const {
        return m_lvl;
    }

    Sink *sink() const {
        return m_sink;
    }

    std::string messageText() const {
        return m_messageStream.str();
    }

    std::ostream &messageStream() {
        return m_messageStream;
    }
private:
    Sink *m_sink;
    LogLevel m_lvl;
    std::string m_tag;
    std::ostringstream m_messageStream;

    void flush() {
        m_sink->sink(*this);
    }
};

class Logger {
public:
    virtual ~Logger() {}

    static std::string levelName(LogLevel lvl) {
        switch(lvl) {
        case LogLevel::DEBUG : return "DEBUG";
        case LogLevel::INFO : return "INFO";
        case LogLevel::WARN : return "WARN";
        case LogLevel::ERROR : return "ERROR";
        default : return "_";
        }
    }

    static std::string levelColor(LogLevel lvl) {
        switch(lvl) {
        case LogLevel::DEBUG : return COLOR_WHITE;
        case LogLevel::INFO : return COLOR_BLUE;
        case LogLevel::WARN : return COLOR_YELLOW;
        case LogLevel::ERROR : return COLOR_RED;
        default: return COLOR_WHITE;
        }
    }

    std::unique_ptr<LogMessage> debug();
    std::unique_ptr<LogMessage> info();
    std::unique_ptr<LogMessage> warn();
    std::unique_ptr<LogMessage> error();

    virtual std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) = 0;
protected:
    Logger() {}
};

// TODO implement sinks and filters
class RootLogger : public Logger {
public:
    RootLogger(Sink *sink) : sink(sink) {}

    std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) {
        return std::unique_ptr<LogMessage>(new LogMessage(sink, lvl, tag));
    }
private:
    Sink *sink;
};

std::unique_ptr<LogMessage> operator << (std::unique_ptr<LogMessage> message, std::ostream& (*pf) (std::ostream&));

template <typename T>
std::unique_ptr<LogMessage> operator << (std::unique_ptr<LogMessage> message, T const& value) {
    if(message.get() == nullptr) {
        std::cerr << "LOG MESSAGE IS NULL" << std::endl;
    } else {
        message->messageStream() << value;
    }

    return message;
}

class ChildLogger : public Logger {
public:
    /**
     * @brief Create a new child logger with the given name and parent.
     * @param name logger's name, will be prepended to the tag
     * @param parent all logging messages will be delegated to this parent
     */
    ChildLogger(const std::string &name, Logger *parent)
        : parent(parent), name(name) {}
private:
    /**
     * @brief Delegate all logging outputs to this parent logger.
     */
    Logger *parent;

    /**
     * @brief Name of the child logger
     */
    std::string name;
protected:
    std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) {
        if(tag.empty()) {
            return parent->log(lvl, name);
        } else {
            std::ostringstream newTag;
            newTag << name << "." << tag;
            return parent->log(lvl, newTag.str());
        }
    }
};

class ConsoleSink : public Sink {
public:
    ConsoleSink(std::ostream& out) : out(out), colored(true), time(true) {}

    void sink(const LogMessage &message) {
        if(time) {
            time_t t = ::time(0);   // get time now
            struct tm * now = localtime( & t );
            out << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << " ";
        }
        if(colored) {
            out << Logger::levelColor(message.level());
        }
        out << Logger::levelName(message.level()) << " " << message.tag();
        if(colored) {
            out << COLOR_WHITE;
        }
        out << ": " << message.messageText() << std::endl;
    }

    ConsoleSink& printTime(bool time);

    ConsoleSink& printColored(bool colored);
private:
    std::ostream& out;
    bool colored;
    bool time;
};

#endif /* LMS_CORE_LOGGER_H */
