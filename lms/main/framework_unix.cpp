#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <algorithm>

#include <lms/loader.h>
#include <lms/framework.h>

#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

namespace lms{

std::string Framework::programDirectory(){
    static std::string directory;

    if(directory.empty()) {
        char path[PATH_MAX];
        memset (path, 0, PATH_MAX);
#ifdef __APPLE__
        uint32_t size = PATH_MAX;
        if( _NSGetExecutablePath(path, &size) == 0 ) {
            char* fullpath = realpath(path, NULL);
            if( !fullpath ) {
                perror("realpath failed");
                exit(1);
            }
            directory = fullpath;
        } else {
            perror("_NSGetExecutablePath failed");
            exit(1);
        }
#else
        if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
            perror("readlink failed");
            exit(1);
        }
        directory = path;
#endif
        directory = directory.substr(0, directory.rfind("/"));
        directory = directory.substr(0, directory.rfind("/"));
        directory = directory + "/";
     }

    //std::cout << "ProgramDirectory: " << directory << std::endl;
    return directory;
}
}
