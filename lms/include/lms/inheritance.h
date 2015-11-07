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
};

template <typename... Args>
struct Impl;

template <>
struct Impl<>
{
    static bool isSubType(size_t hashcode){
        (void)hashcode;
        return false;

  }
};

template <typename First, typename... Args>
struct Impl<First, Args...>
{
    static bool isSubType(size_t hashcode){
        if(hashcode == typeid(First).hash_code()){
            //is subtype
            return true;
        }
        //enroll others
        bool sub = Impl<Args...>::isSubType(hashcode);
        if(sub)
            return true;
        //go deeper in the tree
        if(std::is_base_of<InheritanceBase,First>::value){
            InheritanceBase *inh = (InheritanceBase*)(new First());
            sub = inh->isSubType(hashcode);
            if(sub)
                return true;

        }
        return false;
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
     * @brief isSubType
     * @param hashcode of the given class
     * @return true if the object is a subtype of the class given by it's hashcode
     */
    template<typename... REST>
    bool isSubType(size_t hashcode){
        return Impl<REST...>::isSubType(hashcode);
    }
};



}//lms


#endif //LMS_INHERITANCE_H
