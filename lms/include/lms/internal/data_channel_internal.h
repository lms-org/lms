#ifndef LMS_DATA_CHANNEL_INTERNAL_H
#define LMS_DATA_CHANNEL_INTERNAL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include <typeindex>
#include "lms/logger.h"
#include "lms/serializable.h"
#include "lms/inheritance.h"
#include "lms/extra/type.h"
#include "lms/type_result.h"
#include "lms/any.h"

namespace lms {
namespace internal {

class Runtime; //circle dependency
class ModuleWrapper;

// BACKEND

struct ObjectBase {
    virtual ~ObjectBase() {}
    virtual void* get() =0;
    /**
     * @brief getInheritance we can't cast to void* if we want inheritance <3
     * @return inhertiance*
     */
    virtual Inheritance* getInheritance() = 0;
    virtual Serializable* getSerializable()=0;
    virtual std::string typeName() const =0;
    virtual std::type_index type() const =0;
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
        if(type() == typeid(T)) {
            return TypeResult::SAME;
        }
        //check if the asked object is void
        if(std::is_same<T, lms::Any>::value){
            return TypeResult::SUBTYPE;
        }

        //if the current object is void
        if(isVoid()) {
            return TypeResult::SUPERTYPE;
        }


        //std::cout << "current Object supports inheritance: "<<supportsInheritance() <<std::endl;
        //std::cout << "new Object supports inheritance: "<<std::is_base_of<Inheritance,T>::value <<std::endl;


        //check if the current object supports Inheritance
       // std::cout << "checkType check old type for Inheritance"<<std::endl;
        if(supportsInheritance()){
            Inheritance *inh = getInheritance();
            if(inh->isSubType(typeid(T))){
                return TypeResult::SUBTYPE;
            }
        }
      //check if the new type supports Inheritance
       // std::cout << "checkType check asked type for Inheritance"<<std::endl;
        if(std::is_base_of<Inheritance,T>::value){
            Inheritance* t = (Inheritance*)new T();
            if(t->isSubType(type())){
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
    Inheritance* getInheritance() override {
        return nullptr;
    }
    Serializable* getSerializable() override{
        return nullptr;
    }

    std::string typeName() const override {
        return extra::typeName<T>();
    }

    std::type_index type() const override {
        return typeid(T);
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

    virtual ~FakeObject(){}
};

template<typename T>
struct Object : public FakeObject<T> {
    T value;

    //TODO rename InheritanceCaller :)
    template <typename A, typename B, bool suppInher>
    struct InheritanceCallerGet;

    template <typename A, typename B>
    struct InheritanceCallerGet<A,B, false> {
        static B* call(Object<A> *obj) {
            (void)obj;
            return nullptr;
        }
    };

    template <typename A, typename B>
    struct InheritanceCallerGet<A,B, true> {
        static B* call (Object<A> *obj) {
            return static_cast<B*>(&obj->value);
        }
    };

    void* get() override {
        return &value;
    }

    Inheritance *getInheritance() override{
        return InheritanceCallerGet<T,Inheritance,std::is_base_of<Inheritance,T>::value>::call(this);
    }

    Serializable* getSerializable() override{
        return InheritanceCallerGet<T,Serializable,std::is_base_of<Serializable,T>::value>::call(this);
    }
};

/*
template<typename T>
class ReadDataChannel;
*/
class DataChannelInternal {
public:


    std::unique_ptr<ObjectBase> main;

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
};

}  // namespace internal
}  // namespace lms

#endif //LMS_DATA_CHANNEL_INTERNAL_H
