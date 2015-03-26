#include <fstream>
#include <string>
#include <cstdlib>

#include <lms/type/module_config.h>
#include <lms/extra/string.h>

namespace lms {
namespace type {

template<>
bool ModuleConfig::parse<std::string>(const std::string &src, std::string &dst) {
    dst = src;
    return true;
}

template<>
bool ModuleConfig::parse<bool>(const std::string &src, bool &dst) {
    if(src == "0" || src == "false") {
        dst = false;
        return true;
    }
    if(src == "1" || src == "true") {
        dst = true;
        return true;
    }
    return false;
}

bool ModuleConfig::loadFromFile(const std::string &path) {
    std::ifstream in(path);

    if(! in.is_open()) {
        return false;
    }

    std::string line;

    while(std::getline(in, line)) {
        if(!line.empty() && line[0] != '#') {
            size_t index = line.find_first_of('=');

            if(index != std::string::npos) {
                properties[extra::trim(line.substr(0, index))] = extra::trim(line.substr(index+1));
            }
        }
    }

    in.close();

    return true;
}

bool ModuleConfig::hasKey(const std::string &key) const {
    return properties.count(key) == 1;
}

bool ModuleConfig::empty() const {
    return properties.empty();
}

} // namespace type
} // namespace lms
