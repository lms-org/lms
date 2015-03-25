#ifndef LMS_TYPE_MODULE_CONFIG_H
#define LMS_TYPE_MODULE_CONFIG_H

#include <unordered_map>
#include <sstream>
#include <vector>

namespace lms {
namespace type {

/**
 * @brief ModuleConfig is a property map
 * where modules can load their configuration from.
 *
 * @author Hans Kirchner
 */
class ModuleConfig {
public:
    /* Default contructors and assignment operators */
    ModuleConfig() = default;
    ModuleConfig(const ModuleConfig &) = default;
    ModuleConfig(ModuleConfig &&) = default;
    ModuleConfig& operator= (const ModuleConfig &) = default;
    ModuleConfig& operator= (ModuleConfig &&) = default;

    /**
     * @brief Load a config file from the given path.
     *
     * The format is KEY = VALUE.
     * Empty lines are ignored.
     * Comment lines start with '#'.
     *
     * @param path file path to the config file
     * @return false if the file could not be opened, true otherwise
     */
    bool loadFromFile(const std::string &path);

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
    T get(const std::string &key) const {
        return get(key, T());
    }

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
     * @param defaultValue if the key doesn't exists the defaultValue will be returned
     * @return value of type T
     */
    template<typename T>
    T get(const std::string &key, const T &defaultValue) const {
        const auto it = properties.find(key);
        if(it == properties.end()) {
            return defaultValue;
        } else {
            T result;
            std::istringstream stream(it->second);
            stream >> result;
            return result;
        }
    }

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
    std::vector<T> getArray(const std::string &key) const {
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
            std::istringstream stream(fullValue.substr(pos, nextPos == std::string::npos ?
                                                           nextPos : nextPos - pos));
            // parse the value
            T value;
            stream >> value;

            // add the value to the vector
            array.push_back(value);
        } while(nextPos != std::string::npos);

        return array;
    }

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

} // namespace type
} // namespace lms

#endif /* LMS_TYPE_MODULE_CONFIG_H */
