#include "lms/messaging.h"

namespace lms {

Messaging::Messaging() {}

void Messaging::send(const std::string &command, const std::string &content) {
    // do not allow concurrent access to the send queue
    std::lock_guard<std::mutex> lock(mtx);
    sendQueue[command].push_back(content);
}

const std::list<std::string> &
Messaging::receive(const std::string &command) const {
    MessageQueue::const_iterator it = receiveQueue.find(command);

    if (it != receiveQueue.end()) {
        return it->second;
    } else {
        // we cannot create an empty list here because
        // we would return a reference to temporary
        return emptyStringList;
    }
}

void Messaging::resetQueue() {
    receiveQueue = std::move(sendQueue);
    sendQueue.clear();
}

} // namespace lms
