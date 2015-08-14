#include <lms/extra/file_monitor.h>

#ifdef __APPLE__
    // TODO
#else
    #include <unistd.h>
#endif

//http://www.ibm.com/developerworks/linux/library/l-ubuntu-inotify/index.html

namespace lms {
namespace extra {

FileMonitor::FileMonitor() {
#ifdef __APPLE__
    // TODO
#else
    fd = inotify_init1(IN_NONBLOCK);
    //fd = inotify_init();
#endif
}

FileMonitor::~FileMonitor() {
#ifdef __APPLE__
    // TODO
#else
    close(fd);
#endif
}

FileMonitor::operator bool () const {
#ifdef __APPLE__
    // TODO
    return true;
#else
    return fd != -1;
#endif
}

bool FileMonitor::watch(const std::string &path) {
#ifdef __APPLE__
    time_t lastMod;
    if(lastModified(path, lastMod)) {
        files.push_back(std::make_pair(path, lastMod));
        return true;
    }
    return false;
#else
    int wd = inotify_add_watch(fd, path.c_str(), MASK);
    return wd != -1;
#endif
}

void FileMonitor::unwatchAll() {
#ifdef __APPLE__
    files.clear();
#else
    close(fd);
    fd = inotify_init1(IN_NONBLOCK);
#endif
}

bool FileMonitor::hasChangedFiles() {
#ifdef __APPLE__
    bool changes = false;
    for(File& file : files) {
        time_t newLastMod;
        if(lastModified(file.first, newLastMod)) {
            if(newLastModified != file.second) {
                changes = true;
                file.second = newLastModified;
            }
        }
    }
    return changes;
#else
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
#endif
}

#ifdef __APPLE__
bool FileMonitor::lastModified(const std::string& path, time_t& t) {
    struct stat fileStats;
    if(stat(path.c_str(), &fileStats) == 0) {
        t = fileStats.st_mtime;
        return true;
    }

    return false;
}
#endif

#ifndef __APPLE__
bool FileMonitor::checkEvent(inotify_event *evt) {
    // check for either MODIFY, MOVE_SELF or DELETE
    return (evt->mask & MASK) || (evt->mask & IN_IGNORED);
}
#endif

}  // namespace extra
}  // namespace lms
