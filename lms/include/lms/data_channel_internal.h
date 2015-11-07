#ifndef LMS_DATA_CHANNEL_INTERNAL_H
#define LMS_DATA_CHANNEL_INTERNAL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "lms/logger.h"
#include "lms/serializable.h"
#include "lms/inheritance.h"
namespace lms {
class Runtime; //circle dependency
struct ModuleWrapper;

// BACKEND

struct Void {
    void operator* () {
        throw std::runtime_error("Fick nicht mit dem Ficker.");
    }

    void* operator-> () {
        throw new std::runtime_error("Du bist nicht Robin Hood");
    }
};

/**
* TODO -1 if the type is invalid, 0 if the type of the DataChannel is
* subtype of the given one, 1 if the given type is the same or subtype of
* the DataChannel-type
*/
enum class TypeResult {
    INVALID, SUBTYPE, SUPERTYPE, SAME
};

struct ObjectBase {
    virtual ~ObjectBase() {}
    virtual void* get() =0;
    virtual std::string typeName() const =0;
    virtual size_t hashCode() const =0;
    virtual bool isSerializable() const =0;
    virtual bool isVoid() const =0;
    virtual bool supportsInheritance() const = 0;

    template<typename T>
    TypeResult checkType() {
        if(hashCode() == typeid(T).hash_code()) {
            return TypeResult::SAME;
        }

        if(isVoid()) {
            return TypeResult::SUBTYPE;
        }
        //check if the current object supports Inheritance
        if(supportsInheritance()){
            Inheritance *inh = static_cast<Inheritance*>(get());
            if(inh->isSubType(typeid(T).hash_code())){
                return TypeResult::SUPERTYPE;
            }
        }
        //check if the new type supports Inheritance
        if(std::is_base_of<Inheritance,T>::value){
            void* t = new T();
            if(static_cast<Inheritance*>(t)->isSubType(hashCode())){
                return TypeResult::SUBTYPE;
            }
            delete static_cast<T*>(t); //TODO
        }

        return TypeResult::INVALID;

        // TODO polymorphic data channel types
        /*if(dynamic_cast<T*>(get()) != nullptr) {
            return TypeResult::SUBTYPE;
        }*/

        //if(dynamic_cast<>(new T()))
    }
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

    bool isSerializable() const override {
        return std::is_base_of<Serializable, T>::value;
    }

    bool supportsInheritance() const override{
        return std::is_base_of<Inheritance,T>::value;
    }

    bool isVoid() const override {
        return std::is_same<T, Void>::value;
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
}  //namespace lms

#endif //LMS_DATA_CHANNEL_INTERNAL_H
