#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#include "lms/extra/os.h"

namespace lms {
namespace extra {

std::string username() {

#ifdef _WIN32
    const char* ENV_KEY = "USERNAME";
#elif __APPLE__
    const char* ENV_KEY = "USER";
#else
    const char* ENV_KEY = "LOGNAME";
#endif
    // Don't use getlogin_r on linux here because getenv is more flexible.
    char* logname = getenv(ENV_KEY);
    return logname != nullptr ? logname : "";
}

bool listDir(std::string const& path, std::vector<std::string> &list) {
    DIR *d = opendir(path.c_str());
    struct dirent *dir;

    if(d == nullptr) {
        return false;
    }

    while((dir = readdir(d)) != nullptr) {
        if(std::string(".") != dir->d_name && std::string("..") != dir->d_name) {
            list.push_back(dir->d_name);
        }
    }

    closedir(d);
    return true;
}

FileType fileType(std::string const& path) {
    struct ::stat info;
    if(::stat(path.c_str(), &info) != 0) {
        return FileType::ERROR;
    }

    if(S_ISREG(info.st_mode)) {
        return FileType::REGULAR_FILE;
    } else if(S_ISDIR(info.st_mode)) {
        return FileType::DIRECTORY;
    } else {
        return FileType::OTHER;
    }
}

std::string homepath() {
    const char* home = ::getenv("HOME");
    return home != nullptr ? home : "";
}

void copyTree(std::string const& from, std::string const& to) {
    switch(fileType(from)) {
    case FileType::REGULAR_FILE:
    {
        std::ifstream source(from, std::ios::binary);
        std::ofstream dest(to, std::ios::binary);
        dest << source.rdbuf();
        break;
    }
    case FileType::DIRECTORY:
    {
        mkdir(to.c_str(), 0775);
        std::vector<std::string> children;
        listDir(from, children);
        for(auto const& child : children) {
            copyTree(from + "/" + child, to + "/" + child);
        }
        break;
    }
    default:
        // ignore
        break;
    }
}

}  // namespace extra
}  // namespace lms
