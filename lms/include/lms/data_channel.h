#ifndef LMS_DATA_CHANNEL_H
#define LMS_DATA_CHANNEL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "lms/logger.h"
namespace lms {
class Runtime; //circle dependency
class ModuleWrapper;

// BACKEND

struct ObjectBase {
    virtual ~ObjectBase() {}
    virtual void* get();
    virtual std::string typeName() const;
    virtual size_t hashCode() const;
    virtual bool serializable() const;
};

template<typename T>
struct Object : public ObjectBase {
    T value;

    void* get() override {
        return &value;
    }

    std::string typeName() const override {
        return extra::typeName<T>();
    }

    size_t hashCode() const override {
        return typeid(T).hash_code();
    }

    bool serializable() const override {
        return std::is_base_of<Serializable, T>::value;
    }
};

template<typename T>
class ReadDataChannel;

class DataChannelInternal {
public:
    virtual ~DataChannelInternal() {}

    /**
     * @brief maintainer Runtime that holds the dataChannel
     */
    Runtime *maintainer;
    /**
     * @brief runtimes which need the dataChannel
     */
    std::vector<Runtime*> runtimes;
    /**
     * @brief dataHost runtime that provides the data
     */
    const Runtime *dataHost;
    int m_cycle; //TODO not sure where/how to set it // -> (not const) sets current cycle-count + returns const ->
    /**
     * @brief readers reading modules
     */
    std::vector<std::shared_ptr<ModuleWrapper>> readers;
    /**
     * @brief writers writing modules
     */
    std::vector<std::shared_ptr<ModuleWrapper>> writers;
    /**
     * @brief addRuntime adds Runtime that wants the data
     * @param runtime
     */
    void addRuntime(Runtime *runtime){
        runtimes.push_back(runtime);
    }
    /**
     * @brief clearRuntimes clears the list of depending runtimes
     */
    void clearRuntimes(){
        runtimes.clear();
    }

    bool isReader(std::shared_ptr<ModuleWrapper> module) const{
        return std::find(readers.begin(),readers.end(),module) != readers.end();
    }

    bool isWriter(std::shared_ptr<ModuleWrapper> module) const{
        return std::find(writers.begin(),writers.end(),module) != writers.end();
    }

    bool isReaderOrWriter(std::shared_ptr<ModuleWrapper> module) const{
        return isReader(module) || isWriter(module);
    }




public:
    /**
    * TODO -1 if the type is invalid, 0 if the type of the DataChannel is
    * subtype of the given one, 1 if the given type is the same or subtype of
    * the DataChannel-type
    */
    bool checkType(size_t hashCode) {
        return this->main->hashCode() == hashCode;
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
        return dataHost != nullptr;//TODO not sure if this works
    }
public:
    std::shared_ptr<ObjectBase> main;

    std::vector<DataChannelInternal> m_preBuffer;
    std::vector<DataChannelInternal> m_buffer;

    //called after each cycle of the runtime for each dataChannel
    void bufferCycle(){
        if(buffered()){
        //clears the buffer
        m_buffer.clear();
        }else if(sharesData()){
        //add data to other runtime
        for(Runtime *runtime : runtimes){
            //TODO
            //runtime.addData(this);
            }
        }
    }
};

// FRONTEND

template<typename T>
class DataChannel {
public:
    DataChannel(std::shared_ptr<DataChannelInternal> internal) :
        m_internal(internal) {}
protected:
    std::shared_ptr<DataChannelInternal> m_internal;
public:
};

template<typename T>
class ReadDataChannel : public DataChannel<T> {
public:
    ReadDataChannel(std::shared_ptr<DataChannelInternal> internal) :
        DataChannel<T>(internal) {}

    const T* operator ->() {
        if(this->m_internal->buffered()){
            if(this->m_internal->m_buffer.size() > 0)
                return this->m_internal->m_buffer[this->m_internal->m_buffer.size() -1];
            else
                return nullptr;
        }else{
            return this->m_internal->main.get();
        }
    }
};

template<typename T>
class WriteDataChannel : public DataChannel<T> {
public:
    WriteDataChannel(std::shared_ptr<DataChannelInternal> internal) :
        DataChannel<T>(internal) {}

    T* operator ->(){
        //m_cycle = maintainer->cycleCount();
        return this->m_internal->main.get();
    }
};

}  //namespace lms

#endif //LMS_DATA_CHANNEL_H
