#ifndef LMS_DATAMANAGER_H
#define LMS_DATAMANAGER_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <typeinfo>

#include <lms/module.h>
#include <lms/logger.h>
#include <lms/configurationloader.h>
#include <lms/extra/type.h>

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
friend class ConfigurationLoader;
friend class ExecutionManager;
private:
    logging::ChildLogger logger;

    class PointerWrapper {
    public:
        virtual ~PointerWrapper() {}
        virtual void* get() = 0;
    };

    template<typename T>
    class PointerWrapperImpl : public PointerWrapper {
    public:
        PointerWrapperImpl() {}
        PointerWrapperImpl(const T &data) : data(data) {}
        void* get() { return &data; }
        T data;
    };

    struct DataChannel {
        DataChannel() : dataWrapper(nullptr), dataSize(0), exclusiveWrite(false) {}

        PointerWrapper *dataWrapper; // TODO hier auch unique_ptr möglich
        size_t dataSize; // currently only for idiot checks
        std::string dataTypeName;
        size_t dataHashCode;
        bool exclusiveWrite;
        std::vector<Module*> readers;
        std::vector<Module*> writers;
    };
private:
    std::map<std::string,DataChannel> channels; // TODO check if unordered_map is faster here

    ConfigurationLoader configLoader;
public:
    DataManager(logging::Logger &rootLogger) : logger("DATAMGR", &rootLogger), configLoader(rootLogger) {
    }
    ~DataManager();

    /**
     * @brief Do not allow copies of a data manager instance.
     */
    DataManager(const DataManager&) = delete;

    /**
     * @brief Do not allow assignment copy of a data manager instance.
     */
    DataManager& operator= (const DataManager&) = delete;

    /**
     * @brief Return the data channel with the given name with read permissions
     * or create one if needed.
     *
     * @param module the requesting module
     * @param name data channel name
     * @return const data channel (only reading)
     */
    template<typename T>
    const T*  readChannel(Module *module, const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.dataWrapper == nullptr) {
            logger.warn() << "Module " << module->getName() << " requested read access for "
                << name << std::endl
                << " but the channel was not yet initialized -> Initializing, just for you <3";

            initChannel<T>(channel);
        } else {
            if(! checkType<T>(channel, name)) {
                return nullptr;
            }
        }

        channel.readers.push_back(module);

        return (const T*)channel.dataWrapper->get();
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
    T* writeChannel(Module *module, const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.exclusiveWrite) {
            logger.error() << "Module " << module->getName() << " requested channel " << name << std::endl
                << " with write access, but the channel is already exclusive.";
            return nullptr;
        }

        // if dataPointer is null, then the channel did not exist yet
        if(channel.dataWrapper == nullptr) {
            initChannel<T>(channel);
        } else {
            if(! checkType<T>(channel, name)) {
                return nullptr;
            }
        }

        channel.writers.push_back(module);

        return (T*)channel.dataWrapper->get();
    }

    /**
     * @brief Return the data channel with the given name with write permissions
     * or create one if needed.
     *
     * NOTE: Only module can have the exclusive write permission. If there
     * is atleast one usual writer the there cannot be an exclusive writer.
     *
     * @param module requesting module
     * @param name data channel name
     * @return data channel (reading + writing)
     */
    template<typename T>
    T* exclusiveWriteChannel(Module *module, const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.exclusiveWrite) {
            logger.error() << "Module " << module->getName() << "requested channel " << name << std::endl
                << " with exclusive write access, but the channel is already exclusive.";
            return nullptr;
        }

        if(channel.dataWrapper == nullptr) {
            // create channel if not yet there
            initChannel<T>(channel);
            channel.exclusiveWrite = true;
        } else {
            // check if requested type is the same as in the datachannel
            if(! checkType<T>(channel, name)) {
                return nullptr;
            }

            if(! channel.writers.empty()) {
                logger.error() << "Channel " << name << " has already writers!";
                return nullptr;
            }
        }

        channel.writers.push_back(module);

        return (T*)channel.dataWrapper->get();
    }

    /**
     * @brief Check if a data channel with the given name is
     * currently initialized.
     *
     * @param name data channel name
     * @return true if channel is existing
     */
    bool hasChannel(const std::string &name) const;

    /**
     * @brief Return a configuration object or load it from a
     * config file first.
     *
     * NOTE: The returned object cannot be changed.
     *
     * @param module requesting module
     * @param name name of the config file without the suffix
     * @return module configuration object, returns an empty config if the config wasn't found
     */
    const type::ModuleConfig* getConfig(Module *module, const std::string &name,
                                        const std::vector<std::string> privateDirectories = {});

private:
    /**
     * @brief Return the internal data channel mapping. THIS IS NOT
     * INTENDED TO BE USED IN MODULES.
     *
     * @return datachannel map
     */
    const std::map<std::string,DataChannel>& getChannels() const;

    /**
     * @brief Release all channels
     *
     * This should be called in lms::Module::deinitialize.
     *
     * @param module the module to look for
     */
    void releaseChannelsOf(const Module *module);

    /**
     * @brief Print all channels with their corresponding readers
     * and writers to stdout.
     */
    void printMapping() const;

    /**
     * @brief Set the content of a data channel
     *
     * @param name datachannel name
     * @param data initial content
     */
    template<typename T>
    void setChannel(const std::string &name, const T &data) {
        DataChannel &channel = channels[name];

        // Delete old channel if there was one
        if(channel.dataWrapper != nullptr) {
            delete channel.dataWrapper;
        }

        channel.dataWrapper = new PointerWrapperImpl<T>(data);
        channel.dataSize = sizeof(T);
        channel.dataTypeName = extra::typeName<T>();
        channel.dataHashCode = typeid(T).hash_code();

        // Reset channel
        channel.exclusiveWrite = false;
        channel.readers.clear();
        channel.writers.clear();
    }

    /**
     * @brief Check if requested channel data type T is the same as the data type
     * that is saved in the channel.
     *
     * @param channel the current state of the datachannel
     * @param name data channel name
     * @return true if types are the same, false otherwise
     */
    template<typename T>
    bool checkType(const DataChannel &channel, const std::string &name) {
        // check for hash code of data types
        if(channel.dataHashCode != typeid(T).hash_code()) {
            logger.error() << "Requested wrong data type for channel " << name << std::endl
                << "Channel type is " << channel.dataTypeName << ", requested was " << extra::typeName<T>();
            return false;
        }

        // check for size of data types
        // TODO this is not longer necessary
        if(channel.dataSize != sizeof(T)) {
            logger.error() << "Wrong data size for channel " << name << "!" << std::endl
                << "Requested " << sizeof(T) << " but is " << channel.dataSize;
            return false;
        }

        return true;
    }

    /**
     * @brief Initialize a data channel for usage.
     * The type is implicitly given by the type parameter T.
     *
     * @param channel the data channel to initialize
     */
    template<typename T>
    void initChannel(DataChannel &channel) {
        // allocate memory for type T and call its constructor
        channel.dataWrapper = new PointerWrapperImpl<T>();

        // used for checkType and better error messages
        channel.dataSize = sizeof(T);
        channel.dataTypeName = extra::typeName<T>();
        channel.dataHashCode = typeid(T).hash_code();

        channel.exclusiveWrite = false;
    }

    /**
     * @brief Return a data channel without creating it
     *
     * @param name data channel name
     * @return NULL, if the datachannel was not yet initialized
     * otherwise the data channel object
     */
    template<typename T>
    T* getChannel(const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.dataWrapper == nullptr) {
            return nullptr;
        } else {
            if(! checkType<T>(channel, name)) {
                return nullptr;
            }
        }

        return (T*)channel.dataWrapper->get();
    }
};

}  // namespace lms

#endif /* LMS_CORE_DATAMANAGER_H */
