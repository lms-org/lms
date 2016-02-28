#ifndef LMS_INTERNAL_STRING_H
#define LMS_INTERNAL_STRING_H

#include <string>
#include <vector>

namespace lms {
namespace internal {

/**
 * Remove surrounding whitespace from a std::string.
 * @param s The string to be modified.
 * @param t The set of characters to delete from each end
 * of the string.
 * @return trimmed string
 */
std::string trim(const std::string& str, const std::string &delims = " \t\n\r\f\v");

/**
 * @brief split
 * @param string
 * @param strLength
 * @param splitter
 * @return
 */
std::vector<std::string> split(const char *string, int strLength, char splitter);

std::vector<std::string> split(const std::string &string, char splitter);

/**
 * @brief Returns the directory of a path without trailing slash.
 *
 * /tmp/test.file -> /tmp
 * /tmp/test.file/ -> /tmp
 * tmp/test.file -> tmp
 * / -> /
 *
 * @param path a filesystem path
 * @return filesystem path without basename
 */
std::string dirname(std::string path);

/**
 * @brief Check if a pathname is absolute, meaning it starts with a slash and
 * is not empty.
 * @param path path to check
 * @return true if absolute, false otherwise
 */
bool isAbsolute(const std::string &path);

/**
 * @brief Platform safe implementation for std::getline. \r and \n are handled
 * correctly.
 * @param is input stream
 * @param str line
 * @return the same input stream as given
 */
std::istream& safeGetline(std::istream& is, std::string& str);

/**
 * @brief Check if a string starts with another string.
 * @param str string to investigate
 * @param prefix prefix to look for
 * @return true if str starts with prefix
 */
bool startsWith(std::string const& str, std::string const& prefix);

/**
 * @brief Check if a string ends with another string
 * @param str string to investigate
 * @param suffix suffix to look for
 * @return true if str ends with suffix
 */
bool endsWith(std::string const& str, std::string const& suffix);

std::string versionCodeToString(uint32_t versionCode);

/**
 * @brief Return the length of a string literal.
 *
 * This function computes its result at compile-time.
 *
 * Usage:
 * static_assert(lenOf("test") == 4, "Test should be 4 characters long");
 *
 * @return length of string literal
 */
template<size_t N>
constexpr size_t lenOf(const char (&)[N]) {
    return N - 1;
}

} // namespace internal
} // namespace lms

#endif /* LMS_INTERNAL_STRING_H */
