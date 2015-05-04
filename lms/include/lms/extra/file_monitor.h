#ifndef LMS_EXTRA_FILE_MONITOR_H
#define LMS_EXTRA_FILE_MONITOR_H

#include <string>

#ifdef _WIN32
#elif __APPLE__
#else  // UNIX
#include <sys/inotify.h>
#include <linux/limits.h>
#endif

namespace lms {
namespace extra {

#ifdef _WIN32
    constexpr bool FILE_MONITOR_SUPPORTED = false;
#elif __APPLE__
    constexpr bool FILE_MONITOR_SUPPORTED = false;
#else // UNIX
    constexpr bool FILE_MONITOR_SUPPORTED = true;
#endif

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
    // TODO
#else // UNIX
    int fd;
    static constexpr int BUF_SIZE = sizeof(inotify_event) + NAME_MAX + 1;
    static constexpr std::uint32_t MASK = IN_MODIFY | IN_MOVE_SELF | IN_DELETE_SELF | IN_CREATE;
    bool checkEvent(inotify_event *evt);
#endif
};

}  // namespace extra
}  // namespace lms

#endif /* LMS_EXTRA_FILE_MONITOR_H */
