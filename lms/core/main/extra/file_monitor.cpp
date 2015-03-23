#include <lms/extra/file_monitor.h>
#include <vector>
#include <string>
namespace lms {
namespace extra {

void FileMonitor::addFileToMonitor(std::string path, std::vector<Event> events){
    std::uint8_t tmp = 0;
    for(auto &event:events){
        tmp |= (int)event;
    }
    FileContainer(path,tmp);
}

void FileMonitor::removeFileFromMonitor(std::string path){
    //TODO
}

} //extra

} //lms
