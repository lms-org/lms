#ifndef LMS_CORE_LOGGER_H
#define LMS_CORE_LOGGER_H

#include <iostream>
#include <cstdint>
#include <sstream>
#include <ctime>
#include <memory>
#include <vector>

#include <core/extra/colors.h>

namespace lms{
class LogMessage;
class Module;

/**
 * @brief A logging sink is the abstract concept
 * of a service that is able to write or append a log message
 * to a given target (e.g. terminal, file, network socket)
 *
 * This class is abstract. You must override the sink method
 * in your implementation.
 *
 * @author Hans Kirchner
 */
class Sink {
public:
    /**
     * @brief Virtual destructor. Therefore, can be overridden in subclasses.
     */
    virtual ~Sink() {}

    /**
     * @brief Write or append the given log message to an output.
     * @param message a log message to write
     */
    virtual void sink(const LogMessage &message) = 0;
};

/**
 * @brief All available log levels
 *
 * The log levels should be ordered by their importance.
 *
 * NOTE: This is a C++11 enum class. Look it up if
 * you do not know what that is.
 *
 * @author Hans Kirchner
 */
enum class LogLevel : std::int8_t {
    DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4
};

/**
 * @brief A logging filter can prevent a log message
 * from becoming sinked.
 *
 * NOTE: This class is abstract. You must override filter()
 * in your implementation.
 *
 * @author Hans Kirchner
 */
class LoggingFilter {
public:
    virtual ~LoggingFilter() {}

    /**
     * @brief This method will be called for every log message before being sent to a sink.
     * @param level logging level
     * @param tag logging tag
     * @return true if the log message should be sinked, false if the log message
     * should be ignored.
     */
    virtual bool filter(LogLevel level, const std::string &tag) = 0;
};

/**
 * @brief A log message is not more than a struct consisting
 * of a log level, tag and an appendable log message.
 *
 * A log message is appendable in the following way:
 * message << "Log message No. " << 1;
 *
 * TODO: the sink should not be a usual pointer
 *
 * @author Hans Kirchner
 */
class LogMessage {
public:
    /**
     * @brief Create a new log message with the given values.
     * @param sink Non-null pointer to a sink implementation instance
     * @param lvl logging level
     * @param tag logging tag (used for logging hierarchies and log filtering)
     */
    LogMessage(Sink &sink, LogLevel level, const std::string& tag)
        : tag(tag), level(level), sink(sink) {}

    /**
     * @brief The destructor will flush the log message to the
     * sink given in the constructor.
     *
     * If the log message is wrapped in a unique pointer,
     * this will happen automatically.
     */
    ~LogMessage();

    /**
     * @brief The tag of this log message
     */
    const std::string tag;

    /**
     * @brief The log level (severity) of this log message.
     */
    const LogLevel level;

    /**
     * @brief A reference to a sink instance.
     *
     * This log message will be flushed to
     * that sink in the destructor.
     *
     * TODO is in conflict with RootLogger's Sink unique_ptr
     */
    Sink &sink;

    /**
     * @brief The logging message.
     */
    std::string messageText() const {
        return messageStream.str();
    }

    /**
     * @brief The logging message stream.
     *
     * You can append to this stream via the << operator.
     */
    std::ostringstream messageStream;
};

/**
 * @brief Make the LogMessage appendable.
 */
std::unique_ptr<LogMessage> operator << (std::unique_ptr<LogMessage> message, std::ostream& (*pf) (std::ostream&));

/**
 * @brief Make the LogMessage appendable.
 */
template <typename T>
std::unique_ptr<LogMessage> operator << (std::unique_ptr<LogMessage> message, T const& value) {
    if(! message) {
        // TODO if filter returned false this message will point to NULL
        // std::cerr << "LOG MESSAGE IS NULL" << std::endl;
    } else {
        message->messageStream << value;
    }

    return message;
}

/**
 * @brief A logger is able to receive logging messages
 * and forwards them to logging sinks.
 *
 * General usage:
 * logger.debug() << "Log message with parameters " << 42 << "!";
 *
 * NOTE: This class is abstract.
 *
 * @author Hans Kirchner
 */
class Logger {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Logger() {}

    /**
     * @brief Returns the name of the given log level.
     *
     * Example: LogLevel::DEBUG returns "DEBUG".
     *
     * @param lvl a log level
     * @return string representation of the given log level
     */
    static std::string levelName(LogLevel lvl);

    /**
     * @brief Returns a linux terminal compatible color code
     * suitable for the log level.
     *
     * Example: LogLevel::ERROR returns COLOR_RED
     *
     * @param lvl a log level
     * @return an ANSI escape color code
     */
    static std::string levelColor(LogLevel lvl);

    /**
     * @brief Log a debug message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> debug(const std::string& tag = "");

    /**
     * @brief Log an info message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> info(const std::string& tag = "");

    /**
     * @brief Log a warning message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> warn(const std::string& tag = "");

    /**
     * @brief Log an error message.
     * @see log(lvl, tag)
     */
    std::unique_ptr<LogMessage> error(const std::string& tag = "");

    /**
     * @brief Log a message with the given level and tag.
     *
     * Usually you should use debug(), info(), warn() or error().
     *
     * @param lvl logging level (severity)
     * @param tag logging tag
     * @return an appendable logging message that will be automatically flushed
     */
    virtual std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) = 0;
protected:
    Logger() {}
};

/**
 * @brief Instantiate one root logger per application.
 *
 * The root logger forwards all logging messages to its sink.
 *
 * TODO: implement filters
 *
 * @author Hans Kirchner
 */
class RootLogger : public Logger {
public:
    /**
     * @brief Create a new root logger with the given sink instance.
     *
     * NOTE: The sink instance is managed by this root logger.
     * Do not delete it manually, it will be deleted automatically
     * when the logger is deleted.
     *
     * @param sink a logging sink
     */
    explicit RootLogger(std::unique_ptr<Sink> sink, std::unique_ptr<LoggingFilter> filter);

    /**
     * @brief Create a new root logger with a default console sink.
     *
     * You can set the sink with the sink() method later. The old
     * sink will then be deleted.
     */
    RootLogger();

    ~RootLogger() { std::cout << "Delete RootLogger" << std::endl; }

    /**
     * @brief Do not allow the root logger to be copied.
     */
    RootLogger(const RootLogger &logger) = delete;

    /**
     * @brief Set the new sink for this root logger.
     *
     * The old sink will be deleted.
     *
     * @param sink a sink instance
     */
    void sink(std::unique_ptr<Sink> sink);

    /**
     * @brief Set the new filter for this root logger.
     *
     * The old filter will be deleted.
     *
     * @param filter a filter instance
     */
    void filter(std::unique_ptr<LoggingFilter> filter);

    /**
     * @brief Log a message with the given level and tag.
     * @param lvl logging level
     * @param tag logging tag
     * @return an appendable log message
     */
    std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) override;
private:
     std::unique_ptr<Sink> m_sink;
     std::unique_ptr<LoggingFilter> m_filter;
};

/**
 * @brief A child logger is forwarding all logging
 * messages to its parent.
 *
 * The parent logger can be the root logger or another
 * child logger.
 *
 * @author Hans Kirchner
 */
class ChildLogger : public Logger {
friend Module;
public:
    /**
     * @brief Create a new child logger with the given name and parent.
     *
     * NOTE: if the parent logger gets deleted before the child logger
     * is deleted then that will cause undefined behavior. So make
     * sure you delete all childs first.
     *
     * @param name logger's name, will be prepended to the tag
     * @param parent all logging messages will be delegated to this parent
     */
    ChildLogger(const std::string &name, Logger *parent)
        : parent(parent), name(name) {
        std::cout << "New child logger "<< name << std::endl;
    }

    ~ChildLogger() { std::cout << "Delete child logger " << name << std::endl; }

    std::unique_ptr<LogMessage> log(LogLevel lvl, const std::string& tag) override;
private:
    /**
     * @brief Create a new ChildLogger with NULL parent and an empty name.
     *
     * NOTE: This is only used in lms::Module.
     */
    ChildLogger() : parent(nullptr) {}

    /**
     * @brief Delegate all logging outputs to this parent logger.
     */
    Logger *parent;

    /**
     * @brief Name of the child logger
     */
    std::string name;
};

/**
 * @brief A console sink is a sink implementation that
 * writes all log messages into an std::ostream.
 *
 * The log messages will be prefixed with the time, the logging
 * level and the logging tag.
 *
 * @author Hans Kirchner
 */
class ConsoleSink : public Sink {
public:
    /**
     * @brief Create a new console sink that will write into the given ostream.
     * @param out an ostream instance, e.g. std::cout or std::cerr
     */
    explicit ConsoleSink(std::ostream& out = std::cout) : m_out(out), m_colored(true), m_time(true) {}

    /**
     * @brief Log the given message to the ostream.
     *
     * Format: TIME LEVEL TAG: MESSAGE
     *
     * @param message a log message instance
     */
    void sink(const LogMessage &message) override;

    /**
     * @brief Set to true if the time should be logged.
     * @return this
     */
    ConsoleSink& printTime(bool time);

    /**
     * @brief Set to true if the output should be colored.
     * @return this
     */
    ConsoleSink& printColored(bool colored);
private:
    std::ostream &m_out;
    bool m_colored;
    bool m_time;
};

class PrefixAndLevelFilter : public LoggingFilter {
public:
    PrefixAndLevelFilter() : m_minLevel(LogLevel::DEBUG) {}
    PrefixAndLevelFilter(LogLevel minLevel) : m_minLevel(minLevel) {}

    void addPrefix(const std::string &prefix) {
        m_prefixes.push_back(prefix);
    }

    void minLevel(LogLevel minLevel) {
        m_minLevel = minLevel;
    }

    bool filter(LogLevel level, const std::string &tag) override;

private:
    std::vector<std::string> m_prefixes;
    LogLevel m_minLevel;
};

}
#endif /* LMS_CORE_LOGGER_H */
