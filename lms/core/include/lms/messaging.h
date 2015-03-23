#ifndef LMS_MESSAGING_H
#define LMS_MESSAGING_H

#include <map>
#include <list>

namespace lms {

/**
 * @brief Send and receive string messages inside the core
 * framework and modules.
 */
class Messaging {
public:
    /**
     * @brief Send a message with the specified command and content.
     *
     * The sent message can be received with the receive method
     * if given the same command.
     *
     * @param command message identifier
     * @param content message content
     */
    void send(const std::string &command, const std::string &content = "");

    /**
     * @brief Receive all messages with the given command name that are currently
     * queued.
     *
     * This will not delete any of the queued messages.
     *
     * @param command message identifier
     * @return queued messages
     */
    const std::list<std::string>& receive(const std::string &command) const;

    /**
     * @brief Delete all queued messages.
     *
     * This should be called after each cycle method in the ExecutionManager.
     */
    void resetQueue();
private:
    typedef std::map<std::string, std::list<std::string>> MessageQueue;
    MessageQueue messageQueue;

    const std::list<std::string> emptyStringList;
};

}  // namespace lms

#endif /* LMS_MESSAGING_H */
