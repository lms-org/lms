#ifndef LMS_DATA_CHANNEL_H
#define LMS_DATA_CHANNEL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "lms/logger.h"
namespace lms {
class Runtime; //circle dependency

class DataChannelBase{

public:
    virtual ~DataChannelBase();

    struct DataChannelInformation {
        DataChannelInformation() : dataSize(0), serializable(false),exclusiveWrite(false) {}
        size_t dataSize; // currently only for idiot checks
        std::string dataTypeName;
        size_t dataHashCode;
        bool serializable;
        bool exclusiveWrite;
    };

protected:
    /**
     * @brief info DataChannelInformation
     */
    std::shared_ptr<DataChannelInformation> info;
    /**
     * @brief maintainer Runtime that holds the dataChannel
     */
    std::shared_ptr<const Runtime> maintainer;
    /**
     * @brief runtimes which need the dataChannel
     */
    std::vector<std::shared_ptr<const Runtime>> runtimes;
    /**
     * @brief dataHost runtime that provides the data
     */
    std::shared_ptr<const Runtime> dataHost;
    int m_cycle; //TODO not sure where/how to set it // -> (not const) sets current cycle-count + returns const ->
    /**
     * @brief readers reading modules
     */
    std::vector<std::string> readers;
    /**
     * @brief writers writing modules
     */
    std::vector<std::string> writers;
    /**
     * @brief addRuntime adds Runtime that wants the data
     * @param runtime
     */
    void addRuntime(std::shared_ptr<const Runtime> runtime){
        runtimes.push_back(runtime);
    }
    /**
     * @brief clearRuntimes clears the list of depending runtimes
     */
    void clearRuntimes(){
        runtimes.clear();
    }

    bool isReader(const std::string moduleName) const{
        return std::find(readers.begin(),readers.end(),moduleName) != readers.end();
    }

    bool isWriter(const std::string moduleName) const{
        return std::find(writers.begin(),writers.end(),moduleName) != writers.end();
    }

    bool isReaderOrWriter(const std::string moduleName) const{
        return isReader(moduleName) || isWriter(moduleName);
    }




public:
    /**
     * @brief getCycle
     * @return cycle in which the object was created
     */
    int getCycle() const{
        return m_cycle;
    }

    bool hasReader() const{
        return readers.size() > 0;
    }
    /**
     * @brief hasWriter
     * @return
     */
    bool hasWriter() const{
        return writers.size() > 0;
    }

    /**
     * @brief sharesData
     * @return true if the data is accessed by another runtime
     */
    bool sharesData() const{
        return runtimes.size() > 0;
    }

    /**
     * @brief buffered
     * @return true if it receives data from another runtime (hasWriters returns false!)
     */
    bool buffered() const {
        return dataHost.get() != nullptr;//TODO not sure if this works
    }
};

template<typename T> class DataChannel: public DataChannelBase{
    friend class DataManager; //To call protected methods
protected:
    std::shared_ptr<T*> main;
    std::shared_ptr<std::vector<DataChannel<T>>> m_buffer;

public:

    const std::vector<DataChannel<T>> &buffer() const{
                                //Übergibt alle verfügbaren T*
                                return m_buffer.get();
    }
                                //called after each cycle of the runtime for each dataChannel

    void bufferCycle(){
        if(buffered()){
        //clears the buffer
        m_buffer.get()->clear();
        }else if(sharesData()){
        //add data to other runtime
        for(std::shared_ptr<Runtime> &runtime : runtimes){
            //TODO
            //runtime.addData(this);
            }
        }
    }



    /**
    * @brief TODO
    * @param name data channel name
    * @return -1 if the type is invalid, 0 if the type of the DataChannel is subtype of the given one, 1 if the given type is the same or subtype of the DataChannel-type
    */
    template<typename L>
    int checkType(const std::string &name, logging::Logger &logger) {
        // check for hash code of data types
        if(info->dataHashCode != typeid(L).hash_code()) {
            logger.error() << "Requested wrong data type for channel " << name << std::endl
                           << "Channel type is " << info->dataTypeName << ", requested was " << extra::typeName<T>();
            return -1;
        }

        // check for size of data types
        // TODO this is not longer necessary
        if(info->dataSize != sizeof(T)) {
            logger.error() << "Wrong data size for channel " << name << "!" << std::endl
                           << "Requested " << sizeof(T) << " but is " << info->dataSize;
            return -1;
        }

        return 0;
    }



};


template<typename T> class ReadDataChannel: public DataChannel<T>{
    public:

    const T* operator ->(){
        if(this->buffered()){
            if(this->m_buffer.size() > 0)
                return this->m_buffer[this->m_buffer.size() -1];
            else
                return nullptr;
        }else{
            return this->main.get();
        }
    }

};

template<typename T> class WriteDataChannel: public DataChannel<T>{
    public:

    T* operator ->(){
        //m_cycle = maintainer->cycleCount();
        return this->main.get();
    }
};

}//namespace
#endif //LMS_DATA_CHANNEL_H
