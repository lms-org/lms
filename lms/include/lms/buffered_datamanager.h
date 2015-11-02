#ifndef LMS_BUFFERED_DATAMANAGER_H
#define LMS_BUFFERED_DATAMANAGER_H

#include <vector>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <typeinfo>

#include "lms/extra/type.h"

namespace lms {

/**
 * @brief BufferedDataManager is a thread-safe map of channel names
 * to generic data channels.
 *
 * One data channel can only hold on kind of objects T. This type T is
 * determined the first time someone is pushing or poping from it.
 */
class BufferedDataManager {
public:
    /**
     * @brief Push an object to the given data channel.
     *
     * The data channel will be created if not yet existent.
     *
     * This method is thread-safe and should therefore not interfer with
     * other push or pop calls.
     * @param channelName name to look up
     * @param value object to push into the channel
     * @return false if the type is wrong, true otherwise
     */
    template<typename T>
    bool push(std::string const& channelName, T const& value) {
        std::unique_lock<std::mutex> lock(m_mutex);

        DataChannel &channel = m_dataChannels[channelName];
        initChannelIfNeeded<T>(channel);

        if(checkType<T>(channel)) {
            static_cast<Queue<T>*>(channel.queueWrapper.get())->queue.push_back(value);
            return true;
        }

        return false;
    }

    /**
     * @brief Pop all object pushed to the given data channel.
     *
     * The data channel will be created if not yet existent.
     *
     * This method is thread-safe and should therefore not interfer with
     * other push or pop calls.
     * @param channelName name to look up
     * @param target all previously pushed objected will be stored there,
     *        the target is cleared beforehand
     * @return false if the type is wrong, true otherwise
     */
    template<typename T>
    bool popAll(std::string const& channelName, std::vector<T> & target) {
        std::unique_lock<std::mutex> lock(m_mutex);

        DataChannel &channel = m_dataChannels[channelName];
        initChannelIfNeeded<T>(channel);

        if(checkType<T>(channel)) {
            std::vector<T> &q = static_cast<Queue<T>*>(channel.queueWrapper.get())->queue;
            q.swap(target);
            q.clear();
            //std::move(q.begin(), q.end(), std::back_inserter(target));
            //q.erase(q.begin(), q.end());
            return true;
        }

        return false;
    }

    /**
     * @brief Look up the data channel given by name and return the configured
     * data type if existent.
     * @param channelName name to look up
     * @return channel type or empty string if not initialized
     */
    std::string channelType(std::string const& channelName) const;
private:
    class QueueBase {
    public:
        virtual ~QueueBase() {}
    };

    template<typename T>
    class Queue : public QueueBase {
    public:
        std::vector<T> queue;
    };

    struct DataChannel {
        std::unique_ptr<QueueBase> queueWrapper;
        std::string dataTypeName;
        size_t dataHashCode;
    };

    template<typename T>
    void initChannelIfNeeded(DataChannel &channel) {
        if(! channel.queueWrapper) {
            channel.queueWrapper = std::unique_ptr<Queue<T>>(new Queue<T>());
            channel.dataTypeName = extra::typeName<T>();
            channel.dataHashCode = typeid(T).hash_code();
        }
    }

    template<typename T>
    bool checkType(DataChannel &channel) {
        return channel.dataHashCode == typeid(T).hash_code();
    }

    std::map<std::string, DataChannel> m_dataChannels;
    std::mutex m_mutex;
};

}  // namespace lms

#endif // LMS_BUFFERED_DATAMANAGER_H
