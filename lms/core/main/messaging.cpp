#include "lms/messaging.h"

namespace lms {

void Messaging::send(const std::string &command, const std::string &content) {
    messageQueue[command].push_back(content);
}

const std::list<std::string>& Messaging::receive(const std::string &command) const {
    MessageQueue::const_iterator it = messageQueue.find(command);

    if(it != messageQueue.end()) {
        return it->second;
    } else {
        // we cannot create an empty list here because
        // we would return a reference to temporary
        return emptyStringList;
    }
}

void Messaging::resetQueue() {
    messageQueue.clear();
}

}  // namespace lms
