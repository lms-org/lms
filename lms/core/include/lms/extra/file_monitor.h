#ifndef LMS_EXTRA_FILE_MONITOR_H
#define LMS_EXTRA_FILE_MONITOR_H

#include <string>
#include <cstdint>
#include <vector>
namespace lms {
namespace extra {
/**
 * @brief The FileMonitor class simple class to monitor files
 */
class FileMonitor{
public:
    enum class Event{
        CREATE = 1, READ = 2, MODIFY=4, DELTE=8
    };


    class FileContainer{
        friend class FileMonitor;
        std::string path;
        std::uint8_t events;
        FileContainer(std::string path, std::uint8_t events):path(path),events(events){
        }
        /**
         * @brief addFile executes os specific code to monitor file
         * @return
         */
        bool addFile();
        /**
         * @brief removeFile executes os specific code to monitor file
         * @return
         */
        bool removeFile();
    };

    void addFileToMonitor(std::string path, std::vector<Event> events);

    void removeFileFromMonitor(std::string path);
};

} // namespace extra
} // namespace lms

#endif /* LMS_EXTRA_FILE_MONITOR_H */
