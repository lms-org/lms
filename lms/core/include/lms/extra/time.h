#ifndef LMS_EXTRA_TIME_H
#define LMS_EXTRA_TIME_H

#include <cstdint>
#include <iostream>
#include <ratio>

namespace lms {
namespace extra {

/**
 * @brief Generate timestamps with microsecond precision.
 */
class PrecisionTime {
 public:
    static const PrecisionTime ZERO;

    /**
     * @brief Create a timestamp pointing to
     * a "zero" time.
     */
    PrecisionTime();

    /**
     * @brief Generate a timestamp. The time returned is not guaranteed
     * to be relative to 1/1/1970 or any other historical or local
     * event. The timestamp is only intended to be used for measuring
     * durations.
     *
     * @return a precision time instance
     */
    static PrecisionTime now();

    /**
     * @brief Get the time as microseconds.
     * @return microseconds
     */
    std::int64_t micros() const;

    /**
     * @brief Construct a precision time from given
     * microseconds
     *
     * @param micros microseconds
     * @return precision time instance
     */
    static PrecisionTime fromMicros(std::int64_t micros);

    /**
     * @brief Construct a precision time from given
     * milliseconds.
     *
     * @param millis milliseconds
     * @return precision time instance
     */
    static PrecisionTime fromMillis(std::int64_t millis);

    /**
     * @brief Get time as floating point type
     * 
     * To specify the scale, the first template argument should be a std::ratio
     * template type such as std::milli (defaults to unit-scale, i.e. seconds)
     * 
     * To specify the used floating point type, the optional second template
     * argument can be specified (defaults to single-precision float type)
     * 
     * Example: Return time as double in milliseconds
     *     
     *     t.toFloat<std::milli, double>()
     *     
     * @return time as floating point value (defaults to float in seconds)
     */
    template<class Scale = std::ratio<1,1>, typename T = float>
    T toFloat() const {
        return T( Scale::den * m_micros ) /  T( Scale::num * 1000000L ); 
    }

    /**
     * @brief Sleep the current thread for micros() microseconds.
     *
     * This function uses the best sleep function that
     * is available on the system.
     *
     * @return the remaining time (if interrupted) or zero
     */
    PrecisionTime sleep() const;

    PrecisionTime operator +(const PrecisionTime &t) const;
    PrecisionTime operator -(const PrecisionTime &t) const;
    PrecisionTime& operator +=(const PrecisionTime &t);
    PrecisionTime& operator -=(const PrecisionTime &t);
    PrecisionTime operator * (int scalar) const;
    PrecisionTime operator / (int scalar) const;
    PrecisionTime& operator *= (int scalar);
    PrecisionTime& operator /= (int scalar);
    bool operator >(const PrecisionTime &t) const;
    bool operator >=(const PrecisionTime &t) const;
    bool operator <(const PrecisionTime &t) const;
    bool operator <=(const PrecisionTime &t) const;
    bool operator ==(const PrecisionTime &t) const;
    bool operator !=(const PrecisionTime &t) const;
 private:
    std::int64_t m_micros;

    /**
     * @brief Create a timestamp pointing to the given timestamp
     * in microseconds.
     *
     * @param micros
     */
    explicit PrecisionTime(std::int64_t micros);
};

/**
 * @brief Operator overloading for printing a precision time
 * into a stream. The format is "x us", where x is the time
 * in micro seconds.
 */
std::ostream& operator <<(std::ostream &stream, const PrecisionTime &t);

}  // namespace extra
}  // namespace lms

#endif /* LMS_EXTRA_TIME_H */
