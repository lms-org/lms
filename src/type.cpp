#include <lms/type.h>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace lms {

std::string demangle(const char *name) {
#ifdef __GNUG__
    int status = -4; // some arbitrary value to eliminate the compiler warning

    std::unique_ptr<char, void (*)(void *)> res{
        abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

    return (status == 0) ? res.get() : name;
#else
    // does nothing if not g++
    return name;
#endif
}

} // namespace lms
