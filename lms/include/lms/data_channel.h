#ifndef LMS_DATA_CHANNEL_H
#define LMS_DATA_CHANNEL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "logger.h"
#include "serializable.h"
#include "internal/data_channel_internal.h"

namespace lms {
namespace internal {
class Runtime; //circle dependency
class ModuleWrapper;
}

template<typename T>
class DataChannel {
    //TODO doesn't work friends <3
    template<typename,typename> friend struct InheritanceCallerGet;
public:
    DataChannel(std::shared_ptr<internal::DataChannelInternal> internal) :
        m_internal(internal) {}
protected:
    std::shared_ptr<internal::DataChannelInternal> m_internal;
public:

    std::string name() const{
        return m_internal->name;
    }


    /**
     * return returns SUBTYPE if the current object is a subtype of the given one
     */
    template<typename L> TypeResult checkType(){
        return m_internal->main->checkType<L>();
    }
    template<typename L> bool castableTo(){
        TypeResult r = checkType<L>();
        return r == TypeResult::SAME || r == TypeResult::SUBTYPE;
    }
    bool serialize(std::ostream &os) const {
        // if we would use dynamic_cast here, we hcould remove the serializable
        // flag of data channels, but that is not necessarily faster or better

        if(m_internal->main && m_internal->main->isSerializable()) {
            const Serializable *data = m_internal->main->getSerializable();
            data->lmsSerialize(os);
            return true;
        } else {
            return false;
        }
    }
protected:
    //Util-method
    template <typename A, bool suppInher>
    struct InheritanceCallerGet;

    template <typename A>
    struct InheritanceCallerGet<A, false> {
        static A* call(DataChannel<T> *obj) {
            return (A*)(obj->m_internal->main->get()); //cast to void
        }
    };

    template <typename A>
    struct InheritanceCallerGet<A, true> {
        static A* call (DataChannel<T> *obj) {
            return dynamic_cast<A*>(obj->m_internal->main->getInheritance());//avoid casting to void*
        }
    };


    /**
     * @brier getWithType, used to avoid void* casting
     * returns the object if cast is possible, returns nullptr if the casting if not
     */
    template <typename A>
    A* getWithType_(){
        if(castableTo<A>()){
            return InheritanceCallerGet<A,std::is_base_of<Inheritance,A>::value>::call(this);
        }
        return nullptr;
    }

    /**
     * @brief get returns the contained object, if you have a lms::Void type, use getWithType()
     * @return
     */
    T* get_() {
        if(std::is_same<T, Any>::value){
            return nullptr;
        }
        return InheritanceCallerGet<T,std::is_base_of<Inheritance,T>::value>::call(this);

    }
};

template<typename T>
class ReadDataChannel : public DataChannel<T> {
public:
    ReadDataChannel() : DataChannel<T>(nullptr) {}

    ReadDataChannel(std::shared_ptr<internal::DataChannelInternal> internal) :
        DataChannel<T>(internal) {}
    /**
     * @brief get returns the contained object, if you have a lms::Void type, use getVoid()
     * @return
     */
    /*
    const T* get() {
        //Handle lms::Void pointer
        if(std::is_same<T, Void>::value){
            return nullptr;
        }
        return static_cast<const T*>(this->m_internal->main->get());

        // TODO buffer
        if(this->m_internal->buffered()){
            if(this->m_internal->m_buffer.size() > 0)
                return this->m_internal->m_buffer[this->m_internal->m_buffer.size() -1];
            else
                return nullptr;
        }else{

        }
    }
    */
    const T* get() {
        return this->get_();
    }

    template <typename A>
    const A* getWithType() {
        return DataChannel<T>::template getWithType_<A>();
    }

    const T* operator ->() {
        return this->get();
    }

    const T& operator *() {
        return *this->get();
    }
};

template<typename T>
class WriteDataChannel : public DataChannel<T> {
public:
    WriteDataChannel(std::shared_ptr<internal::DataChannelInternal> internal) :
        DataChannel<T>(internal) {}

    WriteDataChannel() : DataChannel<T>(nullptr) {}

    T* get() {
        return this->get_();
    }

    template <typename A>
    A* getWithType() {
        return DataChannel<T>::template getWithType_<A>();
    }

    T* operator ->(){
        //m_cycle = maintainer->cycleCount();
        return this->get();
    }

    T& operator *() {
        return *this->get();
    }

    bool deserialize(std::istream &is) {
        if(this->m_internal->main && this->m_internal->main->isSerializable()) {
            Serializable *data = this->m_internal->main->getSerializable();
            data->lmsDeserialize(is);
            return true;
        } else {
            return false;
        }
    }
};

}  //namespace lms

#endif //LMS_DATA_CHANNEL_H
