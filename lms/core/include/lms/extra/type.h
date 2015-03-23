#ifndef LMS_EXTRA_TYPE_H
#define LMS_EXTRA_TYPE_H

#include <string>
#include <typeinfo>

/* COPIED FROM: http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname */

namespace lms {
namespace extra {

/**
 * @brief Demangle a type name returned by typeid(t).name().
 *
 * This works only for code compiled with g++. For all other compilers
 * it will just return the unchanged parameter name.
 *
 * @param name mangled name
 * @return demangled name if g++ was used
 */
std::string demangle(const char* name);

/**
 * @brief Get the demangled name of a type
 *
 * @param t parameter with type T
 * @return demangled name
 */
template <class T>
std::string typeName(const T& t) {
    return demangle(typeid(t).name());
}

template <typename T>
std::string typeName() {
    return demangle(typeid(T).name());
}

} // namespace extra
} // namespace lms

#endif /* LMS_EXTRA_TYPE_H */
