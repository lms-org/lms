#include <memory>

#include <lms/logger.h>

namespace lms {
namespace logging {

LogMessage::~LogMessage() {
    ctx.processMessage(*this);
}

std::unique_ptr<LogMessage> operator <<(std::unique_ptr<LogMessage> message, std::ostream& (*pf) (std::ostream&))
{
    if(message) {
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
            message.reset(new LogMessage(message->ctx, message->level, message->tag));
        } else {
            // in case of any other stream modifier (e.g. std::hex)
            // -> just write it into the stringstream
            message->messageStream << pf;
        }
    }
    return message;
}

} // namespace logging
} // namespace lms

