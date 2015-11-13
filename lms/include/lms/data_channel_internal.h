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

/**
 * @brief The Any struct used to access dataChannels for which you don't know the type
 */
struct Any {
    void operator* () {
        throw std::runtime_error("Any is afraid of stars");
    }

    void* operator-> () {
        throw new std::runtime_error("Robin Hood won't give you his arrow");
    }
};

// BACKEND
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

    /**
     *
     * return returns SUBTYPE if the current object is a subtype of the given one (hashcode)
     */
    template<typename T>
    TypeResult checkType() {
        //Check if they are the same
        if(hashCode() == typeid(T).hash_code()) {
            return TypeResult::SAME;
        }
        //check if the asked object is void
        if(std::is_same<T, Any>::value){
            return TypeResult::SUBTYPE;
        }

        //if the current object is void
        if(isVoid()) {
            return TypeResult::SUPERTYPE;
        }

        //check if the current object supports Inheritance
       // std::cout << "checkType check old type for Inheritance"<<std::endl;
        if(supportsInheritance()){
            Inheritance *inh = static_cast<Inheritance*>(get());
            if(inh->isSubType(typeid(T).hash_code())){
                return TypeResult::SUBTYPE;
            }
        }
        //check if the new type supports Inheritance
       // std::cout << "checkType check asked type for Inheritance"<<std::endl;
        if(std::is_base_of<Inheritance,T>::value){
            Inheritance* t = (Inheritance*)new T();
            if(static_cast<Inheritance*>(t)->isSubType(hashCode())){
                return TypeResult::SUPERTYPE;
            }
            delete t;
        }

        return TypeResult::INVALID;
    }
};

template<typename T>
struct FakeObject: public ObjectBase{
    virtual void* get() override {
        //TODO error-handling
        return nullptr;
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
        return std::is_same<T, Any>::value;
    }
};

template<typename T>
struct Object : public FakeObject {
    T value;

    void* get() override {
        return &value;
    }
};


template<typename T>
class ReadDataChannel;

class DataChannelInternal {
public:
    virtual ~DataChannelInternal() {}

    std::string name;

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
            (void)runtime;
            //TODO
            //runtime.addData(this);
            }
        }
    }
};
}  //namespace lms

#endif //LMS_DATA_CHANNEL_INTERNAL_H
