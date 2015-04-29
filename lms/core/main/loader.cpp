#include <iostream>
#include <cstdio>
#include <climits>
#include <dirent.h>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include <lms/loader.h>
#include <lms/module.h>
#include <pugixml.hpp>
#include <lms/logger.h>
#include <lms/framework.h>

namespace lms{

Loader::Loader(logging::Logger &rootLogger) : logger("LOADER", &rootLogger) {
    pathToModules = Framework::programDirectory() + "external/modules/";
}

void Loader::unload(Module* a) {
    delete a;
    // TODO unload dynamic lib
}


std::string Loader::getModulePath(const std::string &libname) {
    // TODO this is linux specific code -> move to loader_unix.cpp
    return "lib" + libname + ".so";
}

}

