#ifndef LMS_INHERITANCE_H
#define LMS_INHERITANCE_H
#include <memory>
namespace lms{

class InheritanceBase{

public:
    /**
     * @brief isSubType
     * @param hashcode of the given class
     * @return true if the object is a subtype of the class given by it's hashcode
     */
    virtual bool isSubType(size_t hashcode) = 0;
    virtual ~InheritanceBase(){}
};

template <typename... Args>
struct Impl;

template <>
struct Impl<>
{
    static bool isSubType(size_t hashcode,const void* obj){
        (void)hashcode;
        (void)obj;
        return false;
  }

    virtual ~Impl(){}
};

template <typename T, bool HasBase>
struct InheritanceCaller;

template <typename T>
struct InheritanceCaller<T, true> {
    static bool call (T* obj,size_t hashcode) {
        return obj->T::isSubType(hashcode);
    }
};

template <typename T>
struct InheritanceCaller<T, false> {
    static bool call (T* obj,size_t hashcode) {
        (void)hashcode;
        (void)obj;
        return false;
    }
};

template <typename First, typename... Args>
struct Impl<First, Args...>
{
    static bool isSubType(size_t hashcode,const void* obj){
        if(hashcode == typeid(First).hash_code()){
            //is subtype
            return true;
        }
        //enroll others
        bool sub = Impl<Args...>::isSubType(hashcode,obj);
        if(sub)
            return true;
        //go deeper in the tree
        sub = InheritanceCaller<First,std::is_base_of<InheritanceBase,First>::value>::call((First*)(obj),hashcode);
        return sub;
  }
};

/**
 * @brief The Inheritance class
 * extend this class to provide dataChannel type-casting and [insert word]*
 * As I don't know the word for here is a small Example
 * A: B{}
 * -> one module accesses A, one accesses B, the dataChannel of type B will be created, module 1 will get dataChannel with type A but the object will be type B.
 */
class Inheritance:public InheritanceBase{
public:

    /**
     * @brief isSubType call the template isSubType function
     * @param hashcode of the given class
     * @return true if the object is a subtype of the class given by it's hashcode
     */
    virtual bool isSubType(size_t hashcode) override= 0; //TODO not sure if we should/need to handle const
    /**
     * TODO: doesn't support abstract classes yet! use hashcode == typeid(EnvironmentObject).hash_code() instead
     * @brief isSubType call this method with all your supertypes in isSubType
     * @param hashcode of the given class
     * @return true if the object is a subtype of the class given by it's hashcode
     */
    template<typename... REST>
    bool isSubType(size_t hashcode) const{ //TODO not sure if we should/need to handle const
        return Impl<REST...>::isSubType(hashcode,this);
    }

    virtual ~Inheritance(){}
};



}//lms


#endif //LMS_INHERITANCE_H
