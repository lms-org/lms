#ifndef LMS_CORE_DATAMANAGER_H
#define LMS_CORE_DATAMANAGER_H

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <core/handle.h>
#include <core/module.h>

namespace lms{

/**
 * @brief The Operation enum used to detect, which module wants to read, write or modify.
 * There can only be one WRITE-Module per channel
 */
enum class Operation {
    READ,
    WRITE,

    // usually there can be multiple readers and writers
    // but if there is on exclusive writer then there cannot be another writer
    EXCLUSIVE_WRITE
};

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
        void* get() { return &data; }
        T data;
    };

    //class

    struct DataChannel {
        DataChannel() : dataWrapper(nullptr), dataSize(0), exclusiveWrite(false) {}

        PointerWrapper *dataWrapper; // TODO hier auch unique_ptr möglich
        size_t dataSize; // currently only for idiot checks
        bool exclusiveWrite;
        std::vector<std::string> readers;
        std::vector<std::string> writers;
    };

    std::map<std::string,DataChannel> channels; // TODO check if unordered_map is faster here
public:
    ~DataManager();

    template<typename T>
    const T* readChannel(const Module *module, const std::string &name);

    template<typename T>
    T* writeChannel(const Module *module, const std::string &name);

    template<typename T>
    T* exclusiveWriteChannel(const Module *module, const std::string &name);

    /**
     * @brief Release all channels
     *
     * This should be called in lms::Module::deinitialize.
     *
     * @param module
     */
    void releaseChannelsBy(const Module *module);
};


}
#endif /* LMS_CORE_DATAMANAGER_H */
