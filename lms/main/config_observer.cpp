#include "lms/config_observer.h"

namespace lms {

void ConfigObserver::listener(ValueChangedListener l) {
    m_valueChangedListener = l;
}

void ConfigObserver::init(const Config *config) {
    m_cachedConfig = *config; // copy the config into the cache
    m_currentConfig = config;
}

void ConfigObserver::changed() {
    for(const std::string &key : m_keys) {
        if(m_currentConfig->get<std::string>(key)
                != m_cachedConfig.get<std::string>(key)) {
            m_valueChangedListener(key);
        }
    }

    m_cachedConfig = *m_currentConfig;
}

void ConfigObserver::keys(const std::vector<std::string> &keys) {
    m_keys = keys;
}

}  // namespace lms
