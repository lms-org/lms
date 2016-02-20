#ifndef LMS_CONFIG_OBSERVER_H
#define LMS_CONFIG_OBSERVER_H

#include "config.h"

#include <functional>
#include <vector>
#include <string>

namespace lms {

/**
 * @brief A ConfigObserver is able to monitor changes in lms::type::ModuleConfig
 * objects an informs a listener function of changed keys.
 */
class ConfigObserver {
public:
    typedef std::function<void (const std::string &name)> ValueChangedListener;

    /**
     * @brief Set the change listener.
     */
    void listener(ValueChangedListener l);

    /**
     * @brief Call this method inside the module's initialize() method.
     * @param config
     */
    void init(const Config *config);

    /**
     * @brief Call this method inside the module's configsChanged() method.
     */
    void changed();

    /**
     * @brief keys Set the config keys to look for.
     * @param keys vector of keys
     */
    void keys(const std::vector<std::string> &keys);
private:
    ValueChangedListener m_valueChangedListener;
    std::vector<std::string> m_keys;

    Config m_cachedConfig;
    const Config *m_currentConfig;
};

}  // namespace lms

#endif /* LMS_CONFIG_OBSERVER_H */
