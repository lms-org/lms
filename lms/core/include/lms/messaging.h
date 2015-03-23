#ifndef LMS_MESSAGING_H
#define LMS_MESSAGING_H

#include <map>
#include <list>
#include <mutex>

namespace lms {

/**
 * @brief Send and receive string messages inside the core
 * framework and modules.
 *
 * Messages will be queued for one cycle and can be received in
 * the next cycle.
 *
 * The class is thread-safe if used correctly: Do not call
 * resetQueue() outside of ExecutionManager. The send/receive
 * methods can be called in the cycle methods of modules.
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
     * @return queued messages (list of message contents)
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

    /* messages that were sent in the previous cycle that can now be received */
    MessageQueue receiveQueue;

    /* messages that were sent in the current cycle */
    MessageQueue sendQueue;

    const std::list<std::string> emptyStringList;

    /* mutex for sendQueue */
    std::mutex mtx;
};

}  // namespace lms

#endif /* LMS_MESSAGING_H */
