#ifndef LMS_INHERITANCE_H
#define LMS_INHERITANCE_H
#include <memory>
namespace lms{

class Inheritance{

public:
    /**
     * @brief isSubType, if you override this method, you can use use Impl<SUPERCLASSES...>::isSubType(hashcode,obj) as return
     * @param hashcode of the given class
     * @return true if the object is a subtype of the class given by it's hashcode
     */
    virtual bool isSubType(size_t hashcode) const = 0;
    virtual ~Inheritance(){}
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
//TODO rename impl
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
        sub = InheritanceCaller<First,std::is_base_of<Inheritance,First>::value>::call((First*)(obj),hashcode);
        return sub;
  }
};




}//lms


#endif //LMS_INHERITANCE_H
