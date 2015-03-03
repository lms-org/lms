#include <core/type/module_config.h>
#include <fstream>
#include <string>
#include <cstdlib>

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
                properties[trim(line.substr(0, index))] = trim(line.substr(index+1));
            }
        }
    }

    in.close();

    return true;
}

std::string ModuleConfig::trim(const std::string& str, const std::string& delims) {
    std::string result(str);
    result.erase(0, result.find_first_not_of(delims));
    result.erase(result.find_last_not_of(delims) + 1);
    return result;
}

} // namespace type
} // namespace lms
