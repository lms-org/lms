#ifndef LMS_EXTRA_TIME_H
#define LMS_EXTRA_TIME_H

#include <cstdint>
#include <iostream>

namespace lms {
namespace extra {

/**
 * @brief Generate timestamps with microsecond precision.
 */
class PrecisionTime {
 public:
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

    PrecisionTime operator +(const PrecisionTime &t);
    PrecisionTime operator -(const PrecisionTime &t);
    PrecisionTime& operator +=(const PrecisionTime &t);
    PrecisionTime& operator -=(const PrecisionTime &t);
    bool operator >(const PrecisionTime &t);
    bool operator >=(const PrecisionTime &t);
    bool operator <(const PrecisionTime &t);
    bool operator <=(const PrecisionTime &t);
    bool operator ==(const PrecisionTime &t);
    bool operator !=(const PrecisionTime &t);
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
