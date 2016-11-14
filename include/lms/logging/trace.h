#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

namespace lms {
namespace logging {

template<typename T>
class Trace {
public:
    Trace() : m_avg(0), m_squaredAvg(0),
        m_min(std::numeric_limits<T>::max()), m_max(0), m_count(0) {}
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
private:
    T m_avg;
    T m_squaredAvg;
    T m_min;
    T m_max;
    unsigned int m_count;
};

}  // namespace logging
}  // namespace lms
