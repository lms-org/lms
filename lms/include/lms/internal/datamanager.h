#ifndef LMS_DATAMANAGER_H
#define LMS_DATAMANAGER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <type_traits>

#include <lms/logger.h>
#include <lms/extra/type.h>
#include <lms/serializable.h>
#include "module_wrapper.h"
#include "lms/extra/dot_exporter.h"
#include "lms/data_channel.h"
#include "lms/deprecated.h"

namespace lms {
namespace internal {

class ExecutionManager;

/**
 * @brief The DataManager manages the creation, access and deletion of
 * data channels.
 *
 * Modules don't have direct acces to a datamanager instance, instead they
 * use readChannel<T> and writeChannel<T>.
 *
 * Suggested channel types: std::array, lms::StaticImage, simple structs
 *
 * @author Hans Kirchner
 */
class DataManager {
    friend class ExecutionManager;

    friend class Framework;

public:
    typedef std::unordered_map<std::string, std::shared_ptr<DataChannelInternal>> ChannelMap;
private:
    logging::Logger logger;
    ExecutionManager &execMgr;
    ChannelMap channels;
public:
    DataManager(Runtime &runtime, ExecutionManager &execMgr);

    /**
     * @brief Do not allow copies of a data manager instance.
     */
    DataManager(const DataManager &) = delete;

    /**
     * @brief Do not allow assignment copy of a data manager instance.
     */
    DataManager &operator=(const DataManager &) = delete;

    /**
     * @brief Helper function that returns initialized data channel objects.
     */
    template<typename T>
    std::shared_ptr<DataChannelInternal> accessChannel(std::shared_ptr<ModuleWrapper> module,
                                                       const std::string &reqName) {
        std::string name = module->getChannelMapping(reqName);
        std::shared_ptr<DataChannelInternal> &channel = channels[name];

        //initChannelIfNeeded<T>(channel);
        //create object
        if (!channel) {
            //logger.debug("accessChannel")<<"creating new dataChannel"<<name<<" to "<< typeid(T).name();
            channel = std::make_shared<DataChannelInternal>();
            channel->maintainer = &m_runtime;

            //check if T is abstract
            if (std::is_abstract<T>::value) {
                channel->main.reset(new FakeObject<T>());
            } else {
                channel->main.reset(new Object<T>());

            }
        } else {
            if (!channel->main) {
                channel->main.reset(new Object<T>());
                logger.error("accessChannel") << "INVALID STATE, channel != null && channel->main == null";
            } else {
                TypeResult typeRes = channel->main->checkType<T>();
                if (typeRes == TypeResult::INVALID) {
                    logger.error("accessChannel") << "INVALID TYPES GIVEN FOR CHANNEL " << name << " currentType "
                    << channel->main->typeName() << " tried to access it with type: " << extra::typeName<T>();
                    return nullptr;
                } else if (typeRes == TypeResult::SUPERTYPE) {
                    //we can "upgrade" the current channel
                    logger.info("accessChannel") << "upgrading channel " << name << " to " << typeid(T).name();
                    //delete old object
                    //create new one
                    channel->main.reset(new Object<T>());
                }
            }
        }

        //TODO lazy
        channel->name = name;
        return channel;
    }

    /**
     * @brief Return the data channel with the given name with read permissions
     * or create one if needed.
     *
     * @param module the requesting module
     * @param name data channel name
     * @return const data channel (only reading)
     */
    template<typename T>
    ReadDataChannel<T> readChannel(std::shared_ptr<ModuleWrapper> module, const std::string &reqName) {
        std::shared_ptr<DataChannelInternal> channel = accessChannel<T>(module, reqName);
        if (!channel->isReaderOrWriter(module)) {
            channel->readers.push_back(module);
            invalidateExecutionManager();
        }
        return channel;
    }

    /**
     * @brief Return the data channel with the given name with write permissions
     * or create one if needed.
     *
     * @param module the requesting module
     * @param name data channel name
     * @return data channel (reading and writing)
     */
    template<typename T>
    WriteDataChannel<T> writeChannel(std::shared_ptr<ModuleWrapper> module, const std::string &reqName) {
        std::shared_ptr<DataChannelInternal> channel = accessChannel<T>(module, reqName);
        if (!channel->isReaderOrWriter(module)) {
            channel->writers.push_back(module);
            invalidateExecutionManager();
        }
        return channel;
    }

    void writeDAG(lms::extra::DotExporter &dot, const std::string &prefix);

    /**
     * @brief Print all channels with their corresponding readers
     * and writers to stdout.
     */
    void printMapping();

    /**
     * @brief Delete all data channels
     */
    void reset();

private:
    Runtime &m_runtime;

    /**
     * @brief Return the internal data channel mapping. THIS IS NOT
     * INTENDED TO BE USED IN MODULES.
     *
     * @return datachannel map
     */
    const ChannelMap &getChannels() const;

    /**
     * @brief Release all channels
     *
     * This should be called in lms::Module::deinitialize.
     *
     * @param module the module to look for
     */
    void releaseChannelsOf(std::shared_ptr<ModuleWrapper> mod);

    /**
     * @brief Invoke invalidate() on the execution manager instance.
     *
     * This cannot be implemented in this header due to cycling header
     * referencing.
     */
    void invalidateExecutionManager();
};

}  // namespace internal
}  // namespace lms

#endif /* LMS_DATAMANAGER_H */
