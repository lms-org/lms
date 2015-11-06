#ifndef LMS_DATA_CHANNEL_H
#define LMS_DATA_CHANNEL_H
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "lms/logger.h"
#include "lms/serializable.h"
#include "data_channel_internal.h"

namespace lms {
class Runtime; //circle dependency
struct ModuleWrapper;

template<typename T>
class DataChannel {
public:
    DataChannel(std::shared_ptr<DataChannelInternal> internal) :
        m_internal(internal) {}
protected:
    std::shared_ptr<DataChannelInternal> m_internal;
public:
    bool serialize(std::ostream &os) {
        // if we would use dynamic_cast here, we could remove the serializable
        // flag of data channels, but that is not necessarily faster or better

        if(m_internal->main && m_internal->main->isSerializable()) {
            const Serializable *data = static_cast<Serializable*>(m_internal->main->get());
            data->lmsSerialize(os);
            return true;
        } else {
            return false;
        }
    }
};

template<typename T>
class ReadDataChannel : public DataChannel<T> {
public:
    ReadDataChannel() : DataChannel<T>(nullptr) {}

    ReadDataChannel(std::shared_ptr<DataChannelInternal> internal) :
        DataChannel<T>(internal) {}

    const T* get() {

        return static_cast<const T*>(this->m_internal->main->get());

        // TODO buffer
        /*if(this->m_internal->buffered()){
            if(this->m_internal->m_buffer.size() > 0)
                return this->m_internal->m_buffer[this->m_internal->m_buffer.size() -1];
            else
                return nullptr;
        }else{

        }*/
    }

    const T* operator ->() {
        return get();
    }

    const T& operator *() {
        return *get();
    }
};

template<typename T>
class WriteDataChannel : public DataChannel<T> {
public:
    WriteDataChannel(std::shared_ptr<DataChannelInternal> internal) :
        DataChannel<T>(internal) {}

    WriteDataChannel() : DataChannel<T>(nullptr) {}

    T* get() {
        return static_cast<T*>(this->m_internal->main->get());
    }

    T* operator ->(){
        //m_cycle = maintainer->cycleCount();
        return get();
    }

    T& operator *() {
        return *get();
    }

    bool deserialize(std::istream &is) {
        if(this->m_internal->main && this->m_internal->main->isSerializable()) {
            Serializable *data = static_cast<Serializable*>(this->m_internal->main->get());
            data->lmsDeserialize(is);
            return true;
        } else {
            return false;
        }
    }
};

}  //namespace lms

#endif //LMS_DATA_CHANNEL_H
