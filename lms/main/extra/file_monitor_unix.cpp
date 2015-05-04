#include <sys/inotify.h>
#include <unistd.h>

#include <lms/extra/file_monitor.h>
//http://www.ibm.com/developerworks/linux/library/l-ubuntu-inotify/index.html

namespace lms {
namespace extra {

FileMonitor::FileMonitor() {
    fd = inotify_init1(IN_NONBLOCK);
    //fd = inotify_init();
}

FileMonitor::~FileMonitor() {
    close(fd);
}

FileMonitor::operator bool () const {
    return fd != -1;
}

bool FileMonitor::watch(const std::string &path) {
    int wd = inotify_add_watch(fd, path.c_str(), MASK);

    return wd != -1;
}

void FileMonitor::unwatchAll() {
    close(fd);
    fd = inotify_init1(IN_NONBLOCK);
}

bool FileMonitor::hasChangedFiles() {
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int retval = select(fd + 1, &rfds, NULL, NULL, &tv);

    if(retval == -1) {
        perror("hasChangedFiles");
    } else if(retval == 0) {
        //printf("select returned 0\n");
    } else if (retval > 0) {
        printf("select > 0\n");

        if(FD_ISSET(fd, &rfds)) {
            char buf[BUF_SIZE];
            int len = 0;
            inotify_event *evt;
            char *p;

            while((len = read(fd, buf, BUF_SIZE)) > 0) {
                if(len == 0) {
                    printf("read returned 0\n");
                }

                for (p = buf; p < buf + len; ) {
                    evt = (inotify_event *) p;

                    if(checkEvent(evt)) {
                        return true;
                    }

                    p += sizeof(inotify_event) + evt->len;
                }
            }
        }
    }

    return false;
}

bool FileMonitor::checkEvent(inotify_event *evt) {
    // check for either MODIFY, MOVE_SELF or DELETE
    return (evt->mask & MASK) || (evt->mask & IN_IGNORED);
}

}  // namespace extra
}  // namespace lms
