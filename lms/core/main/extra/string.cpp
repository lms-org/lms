#include <lms/extra/string.h>

namespace lms {
namespace extra {

std::string trim(const std::string& str, const std::string& delims) {
    std::string result(str);
    result.erase(0, result.find_first_not_of(delims));
    result.erase(result.find_last_not_of(delims) + 1);
    return result;
}

} // namespace extra
} // namespace lms
