#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>

#include <lms/type/module_config.h>
#include <lms/extra/string.h>

namespace lms {
namespace type {

template<>
bool parse<std::string>(const std::string &src,
                                      std::string &dst) {
    dst = src;
    return true;
}

template<>
bool parse<bool>(const std::string &src, bool &dst) {
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
    bool isMultiline = false;
    std::string lineBuffer;

    while(lms::extra::safeGetline(in, line)) {
        if(line.empty()) {
            // ignore empty lines
            continue;
        }

        if(line[0] == '#') {
            // ignore comment lines
            continue;
        }

        bool isCurrentMultiline = line[line.size() - 1] == '\\';
        std::string normalizedLine = isCurrentMultiline ?
                    line.erase(line.size() - 1) : line;

        if(isMultiline) {
            lineBuffer += normalizedLine;
        } else {
            lineBuffer = normalizedLine;
        }

        isMultiline = isCurrentMultiline;

        if(! isMultiline) {
            size_t index = lineBuffer.find_first_of('=');

            if(index != std::string::npos) {
                properties[extra::trim(lineBuffer.substr(0, index))]
                        = extra::trim(lineBuffer.substr(index+1));
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
