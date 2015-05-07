#ifndef LMS_TYPE_MODULE_CONFIG_H
#define LMS_TYPE_MODULE_CONFIG_H

#include <unordered_map>
#include <sstream>
#include <vector>

#include <lms/extra/string.h>

namespace lms {
namespace type {

/**
 * @brief Parse the given string into a type T.
 *
 * @param src string to parse
 * @param dst put the parsed thing here
 * @return true if parsing was successful, otherwise false
 */
template<typename T>
bool parse(const std::string &src, T &dst) {
    std::istringstream is(src);
    return is >> dst;
}

template<>
bool parse<std::string>(const std::string &src, std::string &dst);

template<>
bool parse<bool>(const std::string &src, bool &dst);

/**
 * @brief ModuleConfig is a key-value mapping that can be read by modules to
 * configure themselves.
 *
 * A key is always a string, the value is read as a string but can be parsed to
 * any value: int, bool, double, float, bool... and many more if they implement
 * operator >>.
 *
 * A key is always mapped to one string. That value can be either single value
 * or a comma separated list. Use getArray<T>(key) to parse a list.
 *
 * ModuleConfigs are usually read from *.lconf files. The parsing algorithm is
 * described in loadFromFile(). Loading from files can be stacked, meaning
 * subsequent calls to loadFromFile may result in overwritten key-value pairs
 * if a key was existent from a previous call.
 *
 * Example code:
 * =============
 *
 * ModuleConfig cfg;
 * cfg.loadFromFile("example_module.lconf");
 *
 * // get simple int value, e.g. "12345"
 * int counter = cfg.get<int>("counter");
 *
 * // get boolean value, default value is set to false, e.g. "true"
 * bool enabled = cfg.get<bool>("enabled", false);
 *
 * // get an array, each value is separated by commas, e.g. "1,2,3 , 4"
 * std::vector<int> checkedIds = cfg.getArray<int>("checked");
 *
 * @author Hans Kirchner
 */
class ModuleConfig {
public:
    // Microsoft VisualStudio does not support C++11 Move Semantics
#if ! defined(_MSC_VER)
    /* Default contructors and assignment operators */
    ModuleConfig() = default;
    ModuleConfig(const ModuleConfig &) = default;
    ModuleConfig(ModuleConfig &&) = default;
    ModuleConfig& operator= (const ModuleConfig &) = default;
    ModuleConfig& operator= (ModuleConfig &&) = default;
#endif

    /**
     * @brief Load a config file from the given path.
     *
     * The format is KEY = VALUE. Empty lines are ignored. Comment lines start
     * with '#'. Key-value pairs can continue to the next line if the line ends
     * with '\' (backslash). This is especially useful for lists. Each value
     * in a list needs to be separated by a comma to the next value.
     *
     * @param path file path to the config file
     * @return false if the file could not be opened, true otherwise
     */
    bool loadFromFile(const std::string &path);

    /**
     * @brief Set a config value. Overwrite key if existing.
     *
     * @param key the key to set
     * @param value the value to set
     */
    template<typename T>
    void set(const std::string &key, const T &value);

    /**
     * @brief Return the value by the given config key.
     *
     * If the key is not found the default constructor
     * of T is invoked the that object is returned.
     *
     * If you want to check if a key is in the config file
     * use hasKey()
     *
     * @param key the key to look for
     * @return value of type T
     */
    template<typename T>
    T get(const std::string &key) const;

    /**
     * @brief Return the value by the given config key.
     *
     * If the key is not found the default constructor
     * of T is invoked the that object is returned.
     *
     * If you want to check if a key is in the config file
     * use hasKey()
     *
     * @param key the key to look for
     * @param defaultValue if the key doesn't exists the defaultValue will be
     * returned
     * @return value of type T
     */
    template<typename T>
    T get(const std::string &key, const T &defaultValue) const;

    /**
     * @brief Return a vector of values for the given key.
     *
     * This is similar to the get<T>(...) method, but the
     * value will be separated by commas.
     *
     * @param key the key to look for
     * @return list of values of type T
     */
    template<typename T>
    std::vector<T> getArray(const std::string &key) const;

    /**
     * @brief Check if the given key is available.
     *
     * @param key the key to look for
     * @return true, if the key is found
     */
    bool hasKey(const std::string &key) const;

    /**
     * @brief Check if this configuration object contains
     * no key-value-pairs.
     *
     * @return true if underlying map is empty, false otherwise
     */
    bool empty() const;

private:
    std::unordered_map<std::string, std::string> properties;
};

template<typename T>
void ModuleConfig::set(const std::string &key, const T &value) {
    properties[key] = value;
}

template<typename T>
T ModuleConfig::get(const std::string &key) const {
    return get(key, T());
}

template<typename T>
T ModuleConfig::get(const std::string &key, const T &defaultValue) const {
    const auto it = properties.find(key);
    if(it == properties.end()) {
        return defaultValue;
    } else {
        T result;
        if(parse(it->second, result)) {
            return result;
        } else {
            // if parsing failed take the default value
            return defaultValue;
        }
    }
}

template<typename T>
std::vector<T> ModuleConfig::getArray(const std::string &key) const {
    std::string fullValue(get<std::string>(key));
    std::vector<T> array;

    // if the key/value-pair was not set
    // -> just return empty vector
    if(fullValue.empty()) {
        return array;
    }

    size_t pos, nextPos = -1;

    do {
        pos = nextPos + 1;
        nextPos = fullValue.find(',', pos);

        // slice one value out of the string
        std::string value(lms::extra::trim(fullValue.substr(pos,
            nextPos == std::string::npos ? nextPos : nextPos - pos)));

        // parse the value
        T parsedValue;

        if(parse(value, parsedValue)) {
            // add the value to the vector
            array.push_back(parsedValue);
        }
    } while(nextPos != std::string::npos);

    return array;
}

} // namespace type
} // namespace lms

#endif /* LMS_TYPE_MODULE_CONFIG_H */
