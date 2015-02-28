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

void Framework::setProgrammDirectory(){
    char path[PATH_MAX];
    memset (path, 0, PATH_MAX);
    if (readlink("/proc/self/exe", path, PATH_MAX) == -1) {
        perror("readlink failed");
        exit(1);
    }
    //get programmdirectory
    programmDirectory = path;
    programmDirectory = programmDirectory.substr(0, programmDirectory.rfind("/"));
    programmDirectory = programmDirectory.substr(0, programmDirectory.rfind("/"));
}
}
