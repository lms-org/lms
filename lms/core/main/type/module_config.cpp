#include <fstream>
#include <string>
#include <cstdlib>

#include <lms/type/module_config.h>
#include <lms/extra/string.h>

namespace lms { namespace type {

/**
 * @brief Specialization for string
 * @param key the key to look for
 * @return value of type string
 */
template<>
std::string ModuleConfig::get<std::string>(const std::string &key) const {
    const auto it = properties.find(key);

    if(it == properties.end()) {
        return "";
    } else {
        return it->second;
    }
}

/**
 * @brief Specialization for int
 * @param key the key to look for
 * @return value of type int
 */
template<>
int ModuleConfig::get<int>(const std::string &key) const {
    const auto it = properties.find(key);

    if(it == properties.end()) {
        return 0;
    } else {
        return std::atoi(it->second.c_str());
    }
}

/**
 * @brief Specialization for double
 * @param key the key to look for
 * @return value of type double
 */
template<>
double ModuleConfig::get<double>(const std::string &key) const {
    const auto it = properties.find(key);

    if(it == properties.end()) {
        return 0;
    } else {
        return std::atof(it->second.c_str());
    }
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

} // namespace type
} // namespace lms
