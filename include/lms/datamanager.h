#ifndef LMS_DATAMANAGER_H
#define LMS_DATAMANAGER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <type_traits>

#include "logger.h"
#include "type.h"
#include "serializable.h"
#include "exception.h"
#include "inheritance.h"

namespace lms {

/**
* TODO -1 if the type is invalid, 0 if the type of the DataChannel is
* subtype of the given one, 1 if the given type is the same or subtype of
* the DataChannel-type
*/
enum class TypeResult { INVALID, SUBTYPE, SUPERTYPE, SAME };

/**
 * @brief The Any struct used to access dataChannels for which you don't know
 *the type.
 *
 * This is class is a virtual super class for all data channel types.
 * Nevertheless you should not derive from this class.
 */
struct Any final {
    //TODO add bytebuffer
    void operator*() {
        LMS_EXCEPTION(
            "Cannot call operator* for lms::Any: Any is afraid of stars");
    }

    void *operator->() {
        LMS_EXCEPTION("Cannot call operator-> for lms::Any: Robin Hood won't "
                      "give you his arrow");
    }
};

template <typename Source, typename Target, bool condition> struct CastIf;

template <typename Source, typename Target>
struct CastIf<Source, Target, false> {
    static Target *call(Source *obj) {
        (void)obj;
        return nullptr;
    }
};

template <typename Source, typename Target>
struct CastIf<Source, Target, true> {
    static Target *call(Source *obj) { return static_cast<Target *>(obj); }
};

struct ObjectBase {
    virtual ~ObjectBase() {}
    virtual void *get() = 0;
    /**
     * @brief getInheritance we can't cast to void* if we want inheritance <3
     * @return inhertiance*
     */
    virtual Inheritance *getInheritance() = 0;
    virtual Serializable *getSerializable() = 0;
    virtual std::string typeName() const = 0;
    virtual std::type_index type() const = 0;
    virtual bool isSerializable() const = 0;
    virtual bool isVoid() const = 0;
    virtual bool supportsInheritance() const = 0;

    /**
     *
     * return returns SUBTYPE if the current object is a subtype of the given
     *one (hashcode)
     */
    template <typename T> TypeResult checkType() {
        // Check if they are the same
        if (type() == typeid(T)) {
            return TypeResult::SAME;
        }
        // check if the asked object is void
        if (std::is_same<T, lms::Any>::value) {
            return TypeResult::SUBTYPE;
        }

        // if the current object is void
        if (isVoid()) {
            return TypeResult::SUPERTYPE;
        }

        // std::cout << "current Object supports inheritance:
        // "<<supportsInheritance() <<std::endl;
        // std::cout << "new Object supports inheritance:
        // "<<std::is_base_of<Inheritance,T>::value <<std::endl;

        // check if the current object supports Inheritance
        // std::cout << "checkType check old type for Inheritance"<<std::endl;
        if (supportsInheritance()) {
            Inheritance *inh = getInheritance();
            if (inh->isSubType(typeid(T))) {
                return TypeResult::SUBTYPE;
            }
        }
        // check if the new type supports Inheritance
        // std::cout << "checkType check asked type for Inheritance"<<std::endl;
        if (std::is_base_of<Inheritance, T>::value) {
            Inheritance *t = (Inheritance *)new T();
            if (t->isSubType(type())) {
                return TypeResult::SUPERTYPE;
            }
            delete t;
        }

        return TypeResult::INVALID;
    }
};

template <typename T> struct FakeObject : public ObjectBase {
    virtual void *get() override {
        LMS_EXCEPTION("Cannot call get() on lms::FakeObject");
    }

    virtual Inheritance *getInheritance() override {
        LMS_EXCEPTION("Cannot call getInheritance() on lms::FakeObject");
    }

    virtual Serializable *getSerializable() override {
        LMS_EXCEPTION("Cannot call getSerializable() on lms::FakeObject");
    }

    std::string typeName() const override { return lms::typeName<T>(); }

    std::type_index type() const override { return typeid(T); }

    bool isSerializable() const override {
        return std::is_base_of<Serializable, T>::value;
    }

    bool supportsInheritance() const override {
        return std::is_base_of<Inheritance, T>::value;
    }

    bool isVoid() const override { return std::is_same<T, Any>::value; }

    virtual ~FakeObject() {}
};

template <typename T> struct Object : public FakeObject<T> {
    T value;

    void *get() override { return &value; }

    Inheritance *getInheritance() override {
        return CastIf<T, Inheritance, std::is_base_of<Inheritance, T>::value>::
            call(&this->value);
    }

    Serializable *getSerializable() override {
        return CastIf<
            T, Serializable,
            std::is_base_of<Serializable, T>::value>::call(&this->value);
    }
};

class DataChannelInternal {
public:
    std::unique_ptr<ObjectBase> main;

    virtual ~DataChannelInternal() {}

    std::string name;
};

template <typename T> class DataChannel {
    // TODO doesn't work friends <3
    template <typename, typename> friend struct InheritanceCallerGet;

public:
    DataChannel(std::shared_ptr<DataChannelInternal> internal)
        : m_internal(internal) {}

protected:
    std::shared_ptr<DataChannelInternal> m_internal;

public:
    std::string name() const { return m_internal->name; }

    /**
     * return returns SUBTYPE if the current object is a subtype of the given
     * one
     */
    template <typename L> TypeResult checkType() {
        return m_internal->main->checkType<L>();
    }
    template <typename L> bool castableTo() {
        TypeResult r = checkType<L>();
        return r == TypeResult::SAME || r == TypeResult::SUBTYPE;
    }
    /**
     * @brief isSerializable
     * @param type Serializable type, by default BINARY
     * @return true if the datachannel is Serializable
     */
    bool isSerializable(Serializable::Type type = Serializable::Type::BINARY) const{
        return m_internal->main && m_internal->main->isSerializable(); //TODO use type in isSerializable()
    }

    bool serialize(std::ostream &os,Serializable::Type type = Serializable::Type::BINARY) const {
        // if we would use dynamic_cast here, we hcould remove the serializable
        // flag of data channels, but that is not necessarily faster or better

        if (isSerializable(type)) {
            const Serializable *data = m_internal->main->getSerializable();
            data->lmsSerialize(os);
            return true;
        } else {
            return false;
        }
    }

protected:
    // Util-method
    template <typename A, bool suppInher> struct InheritanceCallerGet;

    template <typename A> struct InheritanceCallerGet<A, false> {
        static A *call(DataChannel<T> *obj) {
            if (obj->m_internal->main->supportsInheritance()) {
                return dynamic_cast<A *>(
                    obj->m_internal->main->getInheritance());
            } else {
                return static_cast<A *>(obj->m_internal->main->get());
            }
        }
    };

    template <typename A> struct InheritanceCallerGet<A, true> {
        static A *call(DataChannel<T> *obj) {
            return static_cast<A *>(
                obj->m_internal->main->get()); // avoid casting to void*
        }
    };

    /**
     * @brier getWithType, used to avoid void* casting
     * returns the object if cast is possible, returns nullptr if the casting if
     * not
     */
    template <typename A> A *getWithType_() {
        if (castableTo<A>()) {
            return InheritanceCallerGet<
                A, std::is_base_of<Inheritance, A>::value>::call(this);
        }
        return nullptr;
    }

    /**
     * @brief get returns the contained object, if you have a lms::Void type,
     * use getWithType()
     * @return
     */
    T *get_() {
        if (std::is_same<T, Any>::value) {
            return nullptr;
        }
        return InheritanceCallerGet<T, std::is_fundamental<T>::value>::call(
            this);
    }
};

template <typename T> class ReadDataChannel : public DataChannel<T> {
public:
    ReadDataChannel() : DataChannel<T>(nullptr) {}

    ReadDataChannel(std::shared_ptr<DataChannelInternal> internal)
        : DataChannel<T>(internal) {}
    /**
     * @brief get returns the contained object, if you have a lms::Void type,
     * use getVoid()
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
                return
    this->m_internal->m_buffer[this->m_internal->m_buffer.size() -1];
            else
                return nullptr;
        }else{

        }
    }
    */
    const T *get() { return this->get_(); }

    template <typename A> const A *getWithType() {
        return DataChannel<T>::template getWithType_<A>();
    }

    const T *operator->() { return this->get(); }

    const T &operator*() { return *this->get(); }
};

template <typename T> class WriteDataChannel : public DataChannel<T> {
public:
    WriteDataChannel(std::shared_ptr<DataChannelInternal> internal)
        : DataChannel<T>(internal) {}

    WriteDataChannel() : DataChannel<T>(nullptr) {}

    T *get() { return this->get_(); }

    template <typename A> A *getWithType() {
        return DataChannel<T>::template getWithType_<A>();
    }

    T *operator->() {
        // m_cycle = maintainer->cycleCount();
        return this->get();
    }

    T &operator*() { return *this->get(); }

    bool deserialize(std::istream &is) {
        if (this->m_internal->main &&
            this->m_internal->main->isSerializable()) {
            Serializable *data = this->m_internal->main->getSerializable();
            data->lmsDeserialize(is);
            return true;
        } else {
            return false;
        }
    }
};

/**
 * @brief The DataManager manages the creation, access and deletion of
 * data channels.
 *
 * Modules don't have direct acces to a datamanager instance, instead they
 * use readChannel<T> and writeChannel<T>.
 *
 * Suggested channel types: std::array, lms::StaticImage, simple structs
 *
 * @author Hans Kirchner
 */
class DataManager {
public:
    typedef std::unordered_map<std::string,
                               std::shared_ptr<DataChannelInternal>> ChannelMap;

private:
    logging::Logger logger;
    ChannelMap channels;

public:
    DataManager();

    /**
     * @brief Do not allow copies of a data manager instance.
     */
    DataManager(const DataManager &) = delete;

    /**
     * @brief Do not allow assignment copy of a data manager instance.
     */
    DataManager &operator=(const DataManager &) = delete;

    /**
     * @brief Helper function that returns initialized data channel objects.
     */
    template <typename T>
    std::shared_ptr<DataChannelInternal>
    accessChannel(const std::string &name) {
        std::shared_ptr<DataChannelInternal> &channel = channels[name];

        // initChannelIfNeeded<T>(channel);
        // create object
        if (!channel) {
            // logger.debug("accessChannel")<<"creating new
            // dataChannel"<<name<<" to "<< typeid(T).name();
            channel = std::make_shared<DataChannelInternal>();

            // check if T is abstract
            if (std::is_abstract<T>::value) {
                channel->main.reset(new FakeObject<T>());
            } else {
                channel->main.reset(new Object<T>());
            }
        } else {
            if (!channel->main) {
                channel->main.reset(new Object<T>());
                LMS_EXCEPTION(
                    "Invalid state: channel is initialized but object is null");
            } else {
                TypeResult typeRes = channel->main->checkType<T>();
                if (typeRes == TypeResult::INVALID) {
                    LMS_EXCEPTION("Tried to access channel " + name + " (" +
                                  channel->main->typeName() + ") as " +
                                  lms::typeName<T>());
                } else if (typeRes == TypeResult::SUPERTYPE) {
                    // we can "upgrade" the current channel
                    // delete old object
                    // create new one
                    channel->main.reset(new Object<T>());
                }
            }
        }

        // TODO lazy
        channel->name = name;
        return channel;
    }

    /**
     * @brief Return the data channel with the given name with read permissions
     * or create one if needed.
     *
     * @param module the requesting module
     * @param name data channel name
     * @return const data channel (only reading)
     */
    template <typename T>
    ReadDataChannel<T> readChannel(const std::string &name) {
        return accessChannel<T>(name);
    }

    /**
     * @brief Return the data channel with the given name with write permissions
     * or create one if needed.
     *
     * @param module the requesting module
     * @param name data channel name
     * @return data channel (reading and writing)
     */
    template <typename T>
    WriteDataChannel<T> writeChannel(const std::string &name) {
        return accessChannel<T>(name);
    }

    /**
     * @brief Delete all data channels
     */
    void reset();

private:
    /**
     * @brief Return the internal data channel mapping. THIS IS NOT
     * INTENDED TO BE USED IN MODULES.
     *
     * @return datachannel map
     */
    const ChannelMap &getChannels() const;
};

} // namespace lms

#endif /* LMS_DATAMANAGER_H */
