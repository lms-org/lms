#ifndef LMS_INTERNAL_OS_H
#define LMS_INTERNAL_OS_H

#include <string>
#include <vector>

namespace lms {
namespace internal {

/**
 * @brief Returns the user's login name.
 *
 * This function uses getenv on all systems. This allows the user to overwrite
 * it as needed. (We don't need the real name, just a unique identifier.)
 *
 * @return current login name
 */
std::string username();

/**
 * @brief List all files and directories in the given directory.
 *
 * The list will not contain '.' and '..'
 *
 * @param path directory's path
 * @param list result list
 * @return
 */
bool listDir(std::string const& path, std::vector<std::string> &list);

enum class FileType {
    REGULAR_FILE, DIRECTORY, OTHER, ERROR
};

/**
 * @brief Request a file's type.
 * @param path relative or absolute path to a file or directory
 * @return file type
 */
FileType fileType(std::string const& path);

/**
 * @brief Returns $HOME on unix systems
 * @return home path of the user ("~")
 */
std::string homepath();

/**
 * @brief Copy a complete folder structure to another location
 */
void copyTree(std::string const& from, std::string const& to);

}  // namespace internal
}  // namespace lms

#endif /* LMS_INTERNAL_OS_H */
