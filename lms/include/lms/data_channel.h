#ifndef LMS_DATA_CHANNEL_H
#define LMS_DATA_CHANNEL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "lms/logger.h"
namespace lms {
class Runtime; //circle dependency

// BACKEND

class DataChannelInternalBase{

public:
    virtual ~DataChannelInternalBase() {}

    struct DataChannelInformation {
        DataChannelInformation() : serializable(false),exclusiveWrite(false) {}
        std::string dataTypeName;
        size_t dataHashCode;
        bool serializable;
        bool exclusiveWrite;
        std::shared_ptr<Runtime> maintainer;
    };

    /**
     * @brief info DataChannelInformation
     */
    DataChannelInformation info;

protected:
    /**
     * @brief maintainer Runtime that holds the dataChannel
     */
    Runtime *maintainer;
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
    * TODO -1 if the type is invalid, 0 if the type of the DataChannel is
    * subtype of the given one, 1 if the given type is the same or subtype of
    * the DataChannel-type
    */
    bool checkType(size_t hashCode) {
        return info.dataHashCode == hashCode;
    }

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

template<typename T>
class ReadDataChannel;

template<typename T>
class DataChannelInternal: public DataChannelInternalBase{
    friend class DataManager; //To call protected methods
protected:
    std::shared_ptr<std::vector<DataChannelInternal<T>>> m_preBuffer;
public:
    DataChannelInternal() {
        // used for checkType and better error messages
        info.dataTypeName = extra::typeName<T>();
        info.dataHashCode = typeid(T).hash_code();
        info.serializable = std::is_base_of<Serializable, T>::value;

        info.exclusiveWrite = false;
    }

    std::shared_ptr<T*> main;
    std::shared_ptr<std::vector<ReadDataChannel<T>>> m_buffer;

    const std::vector<ReadDataChannel<T>> &buffer() const{
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
};

// FRONTEND

template<typename T>
class DataChannel {
protected:
    std::shared_ptr<DataChannelInternal<T>> m_internal;
public:
};

template<typename T>
class ReadDataChannel : public DataChannel<T> {
private:

public:
    const T* operator ->(){
        if(m_internal->buffered()){
            if(m_internal->m_buffer.size() > 0)
                return m_internal->m_buffer[m_internal->m_buffer.size() -1];
            else
                return nullptr;
        }else{
            return m_internal->main.get();
        }
    }
};

template<typename T>
class WriteDataChannel : public DataChannel<T> {
public:
    T* operator ->(){
        //m_cycle = maintainer->cycleCount();
        return m_internal->main.get();
    }
};

}  //namespace lms

#endif //LMS_DATA_CHANNEL_H
