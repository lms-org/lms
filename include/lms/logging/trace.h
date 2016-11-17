#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

#include "lms/time.h"

namespace lms {
namespace logging {

template<typename T>
class Trace {
public:
    Trace() : m_avg(0), m_squaredAvg(0),
        m_min(std::numeric_limits<T>::max()), m_max(0), m_count(0), m_hasBegin(false) {}
    void updateBegin(lms::Time timestamp) {
        m_hasBegin = true;
        m_begin = timestamp;
    }

    void updateEnd(lms::Time timestamp) {
        if(m_hasBegin) {
            update(T((timestamp - m_begin).micros()));
            m_hasBegin = false;
        }
    }

    T avg() const { return m_avg; }
    T min() const { return m_min; }
    T max() const { return m_max; }
    T var() const { return m_squaredAvg - m_avg * m_avg; }
    T std() const { return std::sqrt(var()); }
    unsigned int count() const { return m_count; }
    void reset() {
        m_avg = 0;
        m_squaredAvg = 0;
        m_min = std::numeric_limits<T>::max();
        m_max = 0;
        m_count = 0;
    }
    lms::Time begin() const { return m_begin; }
    bool hasBegin() const { return m_hasBegin; }
private:
    T m_avg;
    T m_squaredAvg;
    T m_min;
    T m_max;
    unsigned int m_count;
    lms::Time m_begin;
    bool m_hasBegin;

    void update(T x) {
        m_count ++; // increment first
        // http://stackoverflow.com/a/1934266
        m_avg += (x - m_avg) / m_count;
        m_squaredAvg += (x * x - m_squaredAvg) / m_count;
        if(x > m_max) {
            m_max = x;
        }
        if(x < m_min) {
            m_min = x;
        }
    }
};

}  // namespace logging
}  // namespace lms
