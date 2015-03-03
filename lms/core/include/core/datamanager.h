#ifndef LMS_CORE_DATAMANAGER_H
#define LMS_CORE_DATAMANAGER_H

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <core/module.h>

namespace lms{

/**
 * @brief The DataManager class
 *
 * Suggested channel types: std::array, lms::StaticImage, simple structs
 */
class DataManager {
private:
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
public:
    struct DataChannel {
        DataChannel() : dataWrapper(nullptr), dataSize(0), exclusiveWrite(false) {}

        PointerWrapper *dataWrapper; // TODO hier auch unique_ptr möglich
        size_t dataSize; // currently only for idiot checks
        bool exclusiveWrite;
        std::vector<Module*> readers;
        std::vector<Module*> writers;
    };
private:
    std::map<std::string,DataChannel> channels; // TODO check if unordered_map is faster here
public:
    ~DataManager();

    template<typename T>
    const T*  readChannel(Module *module, const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.dataWrapper == nullptr) {
            std::cerr << "Channel " << name << " has not yet any writers!" << std::endl;

            channel.dataWrapper = new PointerWrapperImpl<T>();
            channel.dataSize = sizeof(T);
        } else if(channel.dataSize != sizeof(T)) {
            std::cerr << "Channel " << name << " cannot be accessed with wrong type!" << std::endl;
            // TODO do some error handling here
        }

        channel.readers.push_back(module);

        return (const T*)channel.dataWrapper->get();
    }

    template<typename T>
    T* writeChannel(Module *module, const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.exclusiveWrite) {
            std::cerr << "Channel " << name << " is exclusive write!" << std::endl;
            // TODO do some error handling
        }

        // if dataPointer is null, then the channel did not exist yet
        if(channel.dataWrapper == nullptr) {
            channel.dataWrapper = new PointerWrapperImpl<T>();
            channel.dataSize = sizeof(T);
        } else if(channel.dataSize != sizeof(T)) {
            std::cerr << "Channel " << name << " cannot be accessed with wrong type!" << std::endl;
            // TODO do some error handling here
        }
        /*
         * Fails: F:/UserData/Documents/programmieren/c++/LMS/lms/core/include/core/datamanager.h:95: undefined reference to `lms::Module::getName() const'
         *
         */
        channel.writers.push_back(module);

        return (T*)channel.dataWrapper->get();
    }

    template<typename T>
    T* exclusiveWriteChannel(Module *module, const std::string &name) {
        DataChannel &channel = channels[name];

        if(channel.exclusiveWrite) {
            std::cerr << "Channel " << name << " is exclusive write!" << std::endl;
            // TODO do some error handling
        }

        if(channel.dataWrapper == nullptr) {
            channel.dataWrapper = new PointerWrapperImpl<T>();
            channel.dataSize = sizeof(T);
            channel.exclusiveWrite = true;
        } else if(channel.dataSize != sizeof(T)) {
            std::cerr << "Channel " << name << " cannot be accessed with wrong type!" << std::endl;
            // TODO do some error handling here
        } else if(! channel.writers.empty()) {
            std::cerr << "Channel " << name << " has already writers!" << std::endl;
            // TODO do some error handling here
        }

        channel.writers.push_back(module);

        return (T*)channel.dataWrapper->get();
    }

    template<typename T>
    void setChannel(const std::string &name, const T &data) {
        DataChannel &channel = channels[name];

        // Delete old channel if there was one
        if(channel.dataWrapper != nullptr) {
            delete channel.dataWrapper;
        }

        channel.dataWrapper = new PointerWrapperImpl<T>(data);
        channel.dataSize = sizeof(T);

        // Reset channel
        channel.exclusiveWrite = false;
        channel.readers.clear();
        channel.writers.clear();
    }

    bool hasChannel(const std::string &name);

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
};


}
#endif /* LMS_CORE_DATAMANAGER_H */
