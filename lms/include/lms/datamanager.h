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
#include "lms/module_wrapper.h"
#include "lms/extra/dot_exporter.h"
#include "lms/data_channel.h"
#include "lms/deprecated.h"

namespace lms {

class ExecutionManager;

/**
 * @brief The DataManager manages the creation, access and deletion of
 * data channels.
 *
 * Modules can access an instance of this class via this->datamanager().
 *
 * Suggested channel types: std::array, lms::StaticImage, simple structs
 *
 * @author Hans Kirchner
 */
class DataManager {
friend class ExecutionManager;
friend class Framework;
public:
    typedef std::unordered_map<std::string,std::shared_ptr<DataChannelInternal>> ChannelMap;
private:
    logging::Logger logger;
    ExecutionManager &execMgr;
    ChannelMap channels;
public:
    DataManager(Runtime &runtime, ExecutionManager &execMgr);
    ~DataManager();

    /**
     * @brief Do not allow copies of a data manager instance.
     */
    DataManager(const DataManager&) = delete;

    /**
     * @brief Do not allow assignment copy of a data manager instance.
     */
    DataManager& operator= (const DataManager&) = delete;

    template<typename T, typename DataChannelClass, bool isWriter>
    DataChannelClass accessChannel(std::shared_ptr<ModuleWrapper> module, const std::string &reqName) {
        std::string name = module->getChannelMapping(reqName);
        std::shared_ptr<DataChannelInternal> &channel = channels[name];

        //initChannelIfNeeded<T>(channel);
        //create object
        if(!channel) {
            logger.debug("accessChannel")<<"creating new dataChannel"<<name<<" to "<< typeid(T).name();
            channel = std::make_shared<DataChannelInternal>();
            channel->maintainer = &m_runtime;

            //check if T is abstract
            if(std::is_abstract<T>::value){
                channel->main.reset(new FakeObject<T>());
            }else{
                channel->main.reset(new Object<T>());

            }
        }else{
            if(!channel->main) {
                channel->main.reset(new Object<T>());
                logger.error("accessChannel")<<"INVALID STATE, channel != null && channel->main == null";
            }else{
                TypeResult typeRes = channel->main->checkType<T>() ;
                if(typeRes == TypeResult::INVALID) {
                    logger.error("accessChannel")<< "INVALID TYPES GIVEN FOR CHANNEL "<<name <<" currentType "<< channel->main->typeName()<< " tried to access it with type: "<<extra::typeName<T>();
                    return DataChannelClass(nullptr);
                }else if(typeRes == TypeResult::SUPERTYPE){
                    //we can "upgrade" the current channel
                    logger.info("accessChannel")<<"upgrading channel "<<name << " to "<< typeid(T).name();
                    //delete old object
                    //create new one
                    channel->main.reset(new Object<T>());
                }
            }
        }

        if(! channel->isReaderOrWriter(module)) {
            if(isWriter) {
                channel->writers.push_back(module);
            } else {
                channel->readers.push_back(module);
            }
            invalidateExecutionManager();
        }

        //TODO lazy
        channel->name = name;
        return DataChannelClass(channel);
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
        return accessChannel<T, ReadDataChannel<T>, false>(module, reqName);
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
        return accessChannel<T, WriteDataChannel<T>, true>(module, reqName);
    }

    /**
     * @brief Check if a data channel with the given name is
     * currently initialized.
     *
     * NOTE: This function does not use the transparent channel mapping.
     *
     * @param name data channel name
     * @return true if channel is existing
     */
    bool hasChannel(const std::string &name) const;

    /**
     * @brief Check if a data channel with the given name is currently
     * initialized.
     *
     * This function uses transparent channel mapping.
     *
     * @param module the module that calls this method
     * @param name channel name
     * @return true if channel is existing
     */
    bool hasChannel(Module *module, const std::string &name) const;

    void writeDAG(lms::extra::DotExporter &dot, const std::string &prefix);

    /**
     * @brief Set the content of a data channel. This will NOT reset the
     * channel, instead it just sets the data with the assignment operator.
     *
     * NOTE: This function does not use the transparent channel mapping.
     *
     * @param name datachannel name
     * @param data initial content
     */
    template<typename T>
    DEPRECATED
    void setChannel(const std::string &name, const T &data) {
        // TODO move module configs to ModuleWrapper (not used anywhere else)

        /*DataChannel &channel = channels[name];

        if(channel.dataWrapper == nullptr) {
            // initialize channel
            channel.dataSize = sizeof(T);
            channel.dataTypeName = extra::typeName<T>();
            channel.dataHashCode = typeid(T).hash_code();
            channel.serializable = std::is_base_of<Serializable, T>::value;

            channel.dataWrapper = new PointerWrapperImpl<T>(data);
        } else {
            if(! checkType<T>(channel, name)) {
                return;
            }

            PointerWrapperImpl<T> *wrapper = static_cast<PointerWrapperImpl<T>*>(channel.dataWrapper);
            wrapper->set(data);
        }*/

        // Reset channel
        // TODO create a resetChannel method for this code:
//        channel.exclusiveWrite = false;
//        channel.readers.clear();
//        channel.writers.clear();
    }
    /**
     * @brief Return a data channel without creating it
     * Used for hacky casting, for example:
     * A exends B,
     * module Ma aquired channel with A, module Mb only takes channels with type of B -> init module Ma first and you can use this method.
     * //TODO we could check the subtype in checkType()
     * NOTE: This function does not use transparent channel mapping.
     *
     * @param name data channel name
     * @return NULL, if the datachannel was not yet initialized
     * otherwise the data channel object
     */
    template<typename T>
    DEPRECATED
    T* getChannel(const std::string &name, bool ignoreType) {
        // TODO look for usages

        /*if(channels.find(name) == channels.end()){
            logger.warn("getChannel")<<"channel doesn't exist: "<<name;
            return nullptr;
        }
        DataChannel &channel = channels[name];

        if(channel.dataWrapper == nullptr || (!ignoreType && !checkType<T>(channel, name))) {
            return nullptr;
        }

        return static_cast<T*>(channel.dataWrapper->get());*/
        return nullptr;
    }
    /**
     * @brief Return a data channel without creating it
     *
     * NOTE: This function does not use transparent channel mapping.
     *
     * @param name data channel name
     * @return NULL, if the datachannel was not yet initialized
     * otherwise the data channel object
     */
    template<typename T>
    DEPRECATED
    T* getChannel(const std::string &name) {
        return getChannel<T>(name, false);
    }

    /**
     * @brief Print all channels with their corresponding readers
     * and writers to stdout.
     */
    void printMapping();
private:
    Runtime &m_runtime;

    /**
     * @brief Return the internal data channel mapping. THIS IS NOT
     * INTENDED TO BE USED IN MODULES.
     *
     * @return datachannel map
     */
    const ChannelMap& getChannels() const;

    /**
     * @brief Release all channels
     *
     * This should be called in lms::Module::deinitialize.
     *
     * @param module the module to look for
     */
    void releaseChannelsOf(std::shared_ptr<ModuleWrapper> mod);

    /**
     * @brief Initialize a data channel for usage.
     * The type is implicitly given by the type parameter T.
     *
     * @param channel the data channel to initialize
     */
    template<typename T>
    void initChannelIfNeeded(std::shared_ptr<DataChannelInternal>& channel) {
        if(! channel) {
            channel = std::make_shared<DataChannelInternal>();
            channel->maintainer = &m_runtime;
        }

        if(! channel->main) {
            channel->main = std::make_shared<Object<T>>();
        } else {
            if(channel->main->isVoid()) {

            }
        }
    }

    /**
     * @brief Invoke invalidate() on the execution manager instance.
     *
     * This cannot be implemented in this header due to cycling header
     * referencing.
     */
    void invalidateExecutionManager();
};

}  // namespace lms

#endif /* LMS_CORE_DATAMANAGER_H */
