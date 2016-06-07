#ifndef LMS_TYPE_H
#define LMS_TYPE_H

#include <string>
#include <typeinfo>

/* COPIED FROM:
 * http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
 */

namespace lms {

/**
 * @brief Demangle a type name returned by typeid(t).name().
 *
 * This works only for code compiled with g++. For all other compilers
 * it will just return the unchanged parameter name.
 *
 * @param name mangled name
 * @return demangled name if g++ was used
 */
std::string demangle(const char *name);

/**
 * @brief Get the demangled name of a type
 *
 * Usage:
 * std::vector<int> i;
 * assert(typeName(i) == "std::vector<int>");
 *
 * @param t parameter with type T
 * @return demangled name
 */
template <class T> std::string typeName(const T &t) {
    return demangle(typeid(t).name());
}

/**
 * @brief Get the demangled name of a type
 *
 * This function works like typeName(t), except
 * it uses only the template parameter as the argument.
 *
 * **Example code**
 *
 * ~~~~~
 * assert(typeName<std::vector<int>>() == "std::vector<int>");
 * ~~~~~
 *
 * @return demangled name
 */
template <typename T> std::string typeName() {
    return demangle(typeid(T).name());
}

} // namespace lms

#endif /* LMS_TYPE_H */
