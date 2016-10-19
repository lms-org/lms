#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include <lms/config.h>
#include "internal/string.h"

namespace lms {

struct Config::Private {
    std::unordered_map<std::string, std::string> properties;

    template <typename T>
    T get(const std::string &key, const T &defaultValue) const {
        auto it = properties.find(key);
        if (it == properties.end()) {
            return defaultValue;
        } else {
            return internal::string_cast_to<T>(it->second);
        }
    }

    template <typename T>
    T get(const std::string &key) const{
        T def;
        return get(key,def);
    }

    template <typename T> void set(const std::string &key, const T &value) {
        properties[key] = internal::string_cast_from<T>(value);
    }

    template <typename T>
    std::vector<T> getArray(const std::string &key,
                            const std::vector<T> &defaultValue) const {
        auto it = properties.find(key);
        if (it == properties.end()) {
            return defaultValue;
        } else {
            std::vector<T> result;
            for (const auto &parts :
                 lms::internal::split(it->second, ',')) {
                result.push_back(
                    internal::string_cast_to<T>(lms::internal::trim(parts)));
            }
            return result;
        }
    }

    template <typename T>
    std::vector<T> getArray(const std::string &key) const{
        std::vector<T> def;
        return getArray(key,def);
    }

    template <typename T>
    void setArray(const std::string &key, const std::vector<T> &value) {
        std::ostringstream oss;
        for (auto it = value.begin(); it != value.end(); it++) {
            if (it != value.begin()) {
                oss << " ";
            }
            oss << internal::string_cast_from(*it);
        }
        properties[key] = oss.str();
    }
};

Config::Config() : dptr(new Private) {}

Config::~Config() { delete dptr; }

Config::Config(const Config &other) : dptr(new Private(*other.dptr)) {}

void Config::load(std::istream &in) {
    std::string line;
    bool isMultiline = false;
    std::string lineBuffer;

    while (internal::safeGetline(in, line)) {
        if (internal::trim(line).empty() || line[0] == '#') {
            // ignore empty lines and comment lines
            isMultiline = false;
        } else {
            bool isCurrentMultiline = line[line.size() - 1] == '\\';
            std::string normalizedLine =
                isCurrentMultiline ? line.erase(line.size() - 1) : line;

            if (isMultiline) {
                lineBuffer += normalizedLine;
            } else {
                lineBuffer = normalizedLine;
            }

            isMultiline = isCurrentMultiline;
        }

        if (!isMultiline) {
            size_t index = lineBuffer.find_first_of('=');

            if (index != std::string::npos) {
                dfunc()
                    ->properties[internal::trim(lineBuffer.substr(0, index))] =
                    internal::trim(lineBuffer.substr(index + 1));
            }
        }
    }
}

bool Config::loadFromFile(const std::string &path) {
    std::ifstream in(path);

    if (!in.is_open()) {
        return false;
    }

    load(in);
    in.close();
    return true;
}

bool Config::hasKey(const std::string &key) const {
    return dfunc()->properties.count(key) == 1;
}

bool Config::empty() const { return dfunc()->properties.empty(); }

void Config::clear() { dfunc()->properties.clear(); }

// Template specializations get<T>
template <>
std::string Config::get<std::string>(const std::string &key,
                                     const std::string &defaultValue) const {
    return dfunc()->get(key, defaultValue);
}

template <>
int Config::get<int>(const std::string &key, const int &defaultValue) const {
    return dfunc()->get(key, defaultValue);
}

template <>
float Config::get<float>(const std::string &key,
                         const float &defaultValue) const {
    return dfunc()->get(key, defaultValue);
}

template<>
double Config::get<double>(const std::string &key,
                         const double &defaultValue) const {
    return dfunc()->get(key, defaultValue);
}

template <>
bool Config::get<bool>(const std::string &key, const bool &defaultValue) const {
    return dfunc()->get(key, defaultValue);
}

// Template specializations set<T>
template <>
void Config::set<std::string>(const std::string &key,
                              const std::string &value) {
    dfunc()->set(key, value);
}

template <> void Config::set<int>(const std::string &key, const int &value) {
    dfunc()->set(key, value);
}

template <>
void Config::set<float>(const std::string &key, const float &value) {
    dfunc()->set(key, value);
}

template <> void Config::set<bool>(const std::string &key, const bool &value) {
    dfunc()->set(key, value);
}

template <> void Config::set<double>(const std::string &key, const double &value) {
    dfunc()->set(key, value);
}

// Template specializations getArray<T>
template <>
std::vector<std::string> Config::getArray<std::string>(
    const std::string &key,
    const std::vector<std::string> &defaultValue) const {
    return dfunc()->getArray(key, defaultValue);
}

template <>
std::vector<int>
Config::getArray<int>(const std::string &key,
                      const std::vector<int> &defaultValue) const {
    return dfunc()->getArray(key, defaultValue);
}

template <>
std::vector<float>
Config::getArray<float>(const std::string &key,
                        const std::vector<float> &defaultValue) const {
    return dfunc()->getArray(key, defaultValue);
}

template <>
std::vector<bool>
Config::getArray<bool>(const std::string &key,
                       const std::vector<bool> &defaultValue) const {
    return dfunc()->getArray(key, defaultValue);
}

template <>
std::vector<double>
Config::getArray<double>(const std::string &key,
                       const std::vector<double> &defaultValue) const {
    return dfunc()->getArray(key, defaultValue);
}

// Template specializations setArray<T>
template <>
void Config::setArray<std::string>(const std::string &key,
                                   const std::vector<std::string> &value) {
    dfunc()->setArray(key, value);
}

template <>
void Config::setArray<int>(const std::string &key,
                           const std::vector<int> &value) {
    dfunc()->setArray(key, value);
}

template <>
void Config::setArray<float>(const std::string &key,
                             const std::vector<float> &value) {
    dfunc()->setArray(key, value);
}

template <>
void Config::setArray<bool>(const std::string &key,
                            const std::vector<bool> &value) {
    dfunc()->setArray(key, value);
}

template <>
void Config::setArray<double>(const std::string &key,
                            const std::vector<double> &value) {
    dfunc()->setArray(key, value);
}

} // namespace lms
