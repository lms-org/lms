#ifndef LMS_CONFIG_H
#define LMS_CONFIG_H

#include <string>
#include <vector>

namespace lms {

/**
 * @brief Config is a key-value mapping used for module configuration.
 *
 * A key is always a string, the value is read as a string but can be parsed to
 * any value: int, bool, double, float, bool... and many more if they implement
 * operator >>.
 *
 * A key is always mapped to one string. That value can be either single value
 * or a comma separated list. Use getArray<T>(key) to parse a list.
 *
 * Configs are usually read from *.lconf files. The parsing algorithm is
 * described in loadFromFile(). Loading from files can be stacked, meaning
 * subsequent calls to loadFromFile may result in overwritten key-value pairs
 * if a key was existent from a previous call.
 *
 * **Example code**
 *
 * ~~~~~{.cpp}
 * Config cfg;
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
 * ~~~~~
 *
 * @author Hans Kirchner
 */
class Config {
public:
    friend void swap(Config &first, Config &second) {
        std::swap(first.dptr, second.dptr);
    }

    Config();
    ~Config();

    Config(const Config &);

    Config(Config &&other) : dptr(other.dptr) { other.dptr = nullptr; }

    Config &operator=(Config other) {
        swap(*this, other);
        return *this;
    }

    /**
     * @brief Load a config file from the given path.
     *
     * - The general format is KEY = VALUE.
     * - Empty lines are ignored.
     * - Comment lines start with '#'.
     * - Key-value pairs can continue to the next line if the line ends
     *   with '\' (backslash). This is especially useful for lists.
     * - Each value in a list needs to be separated by a comma to the next
     *value.
     *
     * **Example code**
     *
     * ~~~~~{.cpp}
     * Config cfg;
     * cfg.loadFromFile("myconfig.lconf");
     * ~~~~~
     *
     * @param path file path to the config file
     * @return false if the file could not be opened, true otherwise
     */
    bool loadFromFile(const std::string &path);

    /**
     * @brief Load a config from the given stream.
     *
     * The stream is not closed by this method.
     *
     * **Example code**
     * ~~~~~{.cpp}
     * std::ifstream ifs("myfile.lconf");
     *
     * Config cfg;
     * cfg.load(ifs);
     *
     * ifs.close();
     * ~~~~~
     *
     * @param is input stream to read from (will not be closed automatically)
     */
    void load(std::istream &is);

    /**
     * @brief Set a config value. Overwrite key if existing.
     *
     * @param key the key to set
     * @param value the value to set
     */
    template <typename T> void set(const std::string &key, const T &value);

    template <typename T>
    void setArray(const std::string &key, const std::vector<T> &value);

    /**
     * @brief Return the value by the given config key.
     *
     * If the key is not found the default constructor
     * of T is invoked the that object is returned.
     *
     * If you want to check if a key is in the config file
     * use `hasKey()`.
     *
     * @param key the key to look for
     * @param defaultValue if the key doesn't exists the defaultValue will be
     * returned
     * @return value of type T
     */
    template <typename T>
    T get(const std::string &key, const T &defaultValue) const;

    /**
     * @brief Return the value by the given config key.
     *
     * If the key is not found the default constructor
     * of T is invoked the that object is returned.
     *
     * If you want to check if a key is in the config file
     * use `hasKey()`.
     *
     * @param key the key to look for
     * @return value of type T
     */
    template <typename T>
    T get(const std::string &key) const;

    /**
     * @brief Retrieve a config value and split it at commas.
     *
     * The found values will be pushed to the given list. The list is not
     * cleared before writing any values to it.
     *
     * @param key config key to look for
     * @param list values will be push_back'ed there
     */
    template <typename T>
    std::vector<T> getArray(const std::string &key,
                            const std::vector<T> &defaultValue) const;

    /**
     * @brief Retrieve a config value and split it at commas.
     *
     * The found values will be pushed to the given list. The list is not
     * cleared before writing any values to it.
     *
     * @param key config key to look for
     * @param list values will be push_back'ed there
     */
    template <typename T>
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

    /**
     * @brief Delete all properties of this config.
     *
     * empty() will return true afterwards.
     */
    void clear();

private:
    struct Private;
    Private *dptr;
    inline Private *dfunc() { return dptr; }
    inline const Private *dfunc() const { return dptr; }
};

// Template specializations get<T>
template <>
std::string Config::get<std::string>(const std::string &key,
                                     const std::string &defaultValue) const;
template <>
int Config::get<int>(const std::string &key, const int &defaultValue) const;
template <>
float Config::get<float>(const std::string &key,
                         const float &defaultValue) const;
template <>
bool Config::get<bool>(const std::string &key, const bool &defaultValue) const;

// Template specializations set<T>
template <>
void Config::set<std::string>(const std::string &key, const std::string &value);
template <> void Config::set<int>(const std::string &key, const int &value);
template <> void Config::set<float>(const std::string &key, const float &value);
template <> void Config::set<bool>(const std::string &key, const bool &value);

// Template specializations getArray<T>
template <>
std::vector<std::string> Config::getArray<std::string>(
    const std::string &key, const std::vector<std::string> &defaultValue) const;
template <>
std::vector<int>
Config::getArray<int>(const std::string &key,
                      const std::vector<int> &defaultValue) const;
template <>
std::vector<float>
Config::getArray<float>(const std::string &key,
                        const std::vector<float> &defaultValue) const;
template <>
std::vector<bool>
Config::getArray<bool>(const std::string &key,
                       const std::vector<bool> &defaultValue) const;

// Template specializations setArray<T>
template <>
void Config::setArray<std::string>(const std::string &key,
                                   const std::vector<std::string> &value);
template <>
void Config::setArray<int>(const std::string &key,
                           const std::vector<int> &value);
template <>
void Config::setArray<float>(const std::string &key,
                             const std::vector<float> &value);
template <>
void Config::setArray<bool>(const std::string &key,
                            const std::vector<bool> &value);

} // namespace lms

#endif /* LMS_CONFIG_H */
