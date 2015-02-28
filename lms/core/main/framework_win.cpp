//TODO remove unused imports
#include <limits.h>
#include <cstring>
#include <core/framework.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <core/loader.h>
#include <core/module.h>
#include <core/logger.h>
namespace lms{
template<typename _Target>
union converter {
    void* src;
    _Target target;
};
void Framework::setProgrammDirectory(){

    //TODO
    char path[PATH_MAX];
    memset (path, 0, PATH_MAX);
/*    if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
        perror("readlink failed");
        exit(1);
    }*/
    //get programmdirectory
    programm_directory = path;
    programm_directory = programm_directory.substr(0, programm_directory.rfind("/"));
    programm_directory = programm_directory.substr(0, programm_directory.rfind("/"));
    Framework::programmDirectory = programm_directory;
}
