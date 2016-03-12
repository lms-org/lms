#ifndef LMS_INHERITANCE_H
#define LMS_INHERITANCE_H

#include <memory>
#include <typeindex>

namespace lms{

class Inheritance{

public:
    /**
     * @brief isSubType, if you override this method, you can use use Impl<SUPERCLASSES...>::isSubType(hashcode,obj) as return
     * @param hashcode of the given class
     * @return true if the object is a subtype of the class given by it's hashcode
     */
    virtual bool isSubType(std::type_index) const = 0;
    virtual ~Inheritance(){}
};

template <typename T, bool HasBase>
struct InheritanceCaller;

template <typename T>
struct InheritanceCaller<T, true> {
    static bool call (const T* obj, std::type_index type) {
        return obj->T::isSubType(type);
    }
};

template <typename T>
struct InheritanceCaller<T, false> {
    static bool call (const T*, std::type_index) {
        return false;
    }
};

template <typename... Args>
struct Extends;

template <>
struct Extends<>
{
    template<typename This>
    static bool isSubType(const This*, std::type_index) {
        return false;
    }
};

template <typename Head, typename... Tail>
struct Extends<Head, Tail...>
{
    template<typename This>
    static bool isSubType(const This* obj, std::type_index type) {
        constexpr bool hasInheritance = std::is_base_of<Inheritance,Head>::value;

        return type == typeid(Head)
            || Extends<Tail...>::isSubType(obj, type)
            || InheritanceCaller<Head,hasInheritance>::call(static_cast<const Head*>(obj),type);
    }
};

}//lms


#endif //LMS_INHERITANCE_H
