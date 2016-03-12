#ifndef LMS_INTERNAL_FILE_MONITOR_H
#define LMS_INTERNAL_FILE_MONITOR_H

#include <string>

#ifdef _WIN32
#elif __APPLE__
#include <vector>
#include <ctime>
#include <utility>
#include <sys/stat.h>
#else  // UNIX
#include <sys/inotify.h>
#include <linux/limits.h>
#endif

namespace lms {
namespace extra {

/**
 * @brief The FileMonitor class simple class to monitor files
 */
class FileMonitor {
public:
    FileMonitor();

    ~FileMonitor();

    operator bool () const;

    bool watch(const std::string &path);

    void unwatchAll();

    bool hasChangedFiles();
private:
#ifdef _WIN32
    // TODO
#elif __APPLE__
    typedef std::pair<std::string, time_t> File;
    std::vector<File> files;
    bool lastModified(const std::string& path, time_t& t);
#else // UNIX
    int fd;
    static constexpr int BUF_SIZE = sizeof(inotify_event) + NAME_MAX + 1;
    static constexpr std::uint32_t MASK = IN_MODIFY | IN_MOVE_SELF | IN_DELETE_SELF | IN_CREATE;
    bool checkEvent(inotify_event *evt);
#endif
};

}  // namespace extra
}  // namespace lms

#endif /* LMS_INTERNAL_FILE_MONITOR_H */
