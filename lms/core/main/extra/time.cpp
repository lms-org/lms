#include <iostream> // only for "Not Implemented" messages

#ifdef _WIN32
#elif __APPLE__
    #include <mach/mach.h>
    #include <mach/clock.h>
#else // unix
    #include <time.h>
#endif

#include "lms/extra/time.h"

namespace lms {
namespace extra {

#ifdef _WIN32

PrecisionTime PrecisionTime::now() {
    std::cerr << "PrecisionTime::now not implemented on Win32" << std::endl;
    return PrecisionTime(0);
}

#elif __APPLE__

PrecisionTime PrecisionTime::now() {
    mach_timespec_t time;
    clock_serv_t cclock;
    
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &time);
    mach_port_deallocate(mach_task_self(), cclock);
    
    return PrecisionTime(time.tv_sec * USEC_PER_SEC + time.tv_nsec / NSEC_PER_USEC);
}

#else // unix

PrecisionTime PrecisionTime::now() {
    timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return PrecisionTime(time.tv_sec * 1000000 + time.tv_nsec / 1000);
}

#endif

PrecisionTime::PrecisionTime(std::int64_t micros) : m_micros(micros) {
}

PrecisionTime::PrecisionTime() : m_micros(0) {
}

std::int64_t PrecisionTime::micros() const {
    return m_micros;
}

PrecisionTime PrecisionTime::fromMicros(std::int64_t micros) {
    return PrecisionTime(micros);
}

PrecisionTime PrecisionTime::fromMillis(std::int64_t millis) {
    return PrecisionTime(millis * 1000);
}

PrecisionTime& PrecisionTime::operator +=(const PrecisionTime &t) {
    m_micros += t.micros();
    return *this;
}

PrecisionTime& PrecisionTime::operator -=(const PrecisionTime &t) {
    m_micros -= t.micros();
    return *this;
}

PrecisionTime PrecisionTime::operator +(const PrecisionTime &t) {
    return PrecisionTime(micros() + t.micros());
}

PrecisionTime PrecisionTime::operator -(const PrecisionTime &t) {
    return PrecisionTime(micros() - t.micros());
}

bool PrecisionTime::operator >(const PrecisionTime &t) {
    return micros() > t.micros();
}

bool PrecisionTime::operator >=(const PrecisionTime &t) {
    return micros() >= t.micros();
}

bool PrecisionTime::operator <(const PrecisionTime &t) {
    return micros() < t.micros();
}

bool PrecisionTime::operator <=(const PrecisionTime &t) {
    return micros() <= t.micros();
}

bool PrecisionTime::operator ==(const PrecisionTime &t) {
    return micros() == t.micros();
}

bool PrecisionTime::operator !=(const PrecisionTime &t) {
    return micros() != t.micros();
}

std::ostream& operator <<(std::ostream &stream, const PrecisionTime &t) {
    stream << t.micros() << " us";
    return stream;
}

}  // namespace extra
}  // namespace lms
