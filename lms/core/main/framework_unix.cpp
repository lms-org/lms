//TODO remove unused imports
#include <limits.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <core/loader.h>
#include <core/module.h>
#include <core/logger.h>
#include <core/framework.h>
namespace lms{

std::string Framework::programDirectory(){
    static std::string directory;

    if(directory.empty()) {
        char path[PATH_MAX];
        memset (path, 0, PATH_MAX);
        if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
            perror("readlink failed");
            exit(1);
        }
        //get programmdirectory
        // TODO optimize this a bit
        directory = path;
        directory = directory.substr(0, directory.rfind("/"));
        directory = directory.substr(0, directory.rfind("/"));
     }

    std::cout << "ProgramDirectory: " << directory << std::endl;

     //return directory; // TODO did not work with this line
    return "";
}
}
