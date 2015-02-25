#include <core/logger.h>

std::unique_ptr<LogMessage> Logger::debug() {
    return log(LogLevel::DEBUG, "");
}

std::unique_ptr<LogMessage> Logger::info() {
    return log(LogLevel::INFO, "");
}

std::unique_ptr<LogMessage> Logger::warn() {
    return log(LogLevel::WARN, "");
}

std::unique_ptr<LogMessage> Logger::error() {
    return log(LogLevel::ERROR, "");
}

ConsoleSink& ConsoleSink::printTime(bool time) {
    this->time = time;
    return *this;
}

ConsoleSink& ConsoleSink::printColored(bool colored) {
    this->colored = colored;
    return *this;
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
            message.reset(new LogMessage(message->sink(), message->level(), message->tag()));
        } else {
            // in case of any other stream modifier (e.g. std::hex)
            // -> just write it into the stringstream
            message->messageStream() << pf;
        }
    }
    return message;
}
