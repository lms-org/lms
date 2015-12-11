#include <iostream> // only for "Not Implemented" messages

#ifdef _WIN32
#elif __APPLE__
    #include <mach/mach.h>
    #include <mach/clock.h>
    #include <time.h>
#else // unix
    #include <time.h>
    #include <features.h>
#endif

#include "lms/time.h"

namespace lms {

std::string currentTimeString() {
    char str[50];

    time_t rawtime;
    time (&rawtime);
    tm *t = gmtime(&rawtime);
    snprintf(str, sizeof(str), "%04i%02i%02i-%02i%02i%02i",
        t->tm_year+1900,
        t->tm_mon+1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec
    );

    return str;
}

#ifdef _WIN32

Time Time::now() {
    std::cerr << "Time::now not implemented on Win32" << std::endl;
    return Time();
}

Time Time::sleep() const {
    std::cerr << "Time::sleep not implemented on Win32" << std::endl;
    return Time();
}

#elif __APPLE__

Time Time::now() {
    mach_timespec_t time;
    clock_serv_t cclock;
    
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &time);
    mach_port_deallocate(mach_task_self(), cclock);
    
    return Time(time.tv_sec * USEC_PER_SEC + time.tv_nsec / NSEC_PER_USEC);
}

#else // unix

Time Time::now() {
    // http://linux.die.net/man/2/clock_gettime
    timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return Time::fromMicros(time.tv_sec * 1000000 + time.tv_nsec / 1000);
}

#endif

#ifndef _WIN32
Time Time::sleep() const {
    timespec request, remaining;
    request.tv_sec = m_micros / 1000000;
    request.tv_nsec = (m_micros - request.tv_sec * 1000000) * 1000;
    
#if _XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L
    // http://linux.die.net/man/2/clock_nanosleep
    int result = clock_nanosleep(CLOCK_MONOTONIC, 0, &request, &remaining);
#elif _POSIX_C_SOURCE >= 199309L || __APPLE__
    // http://linux.die.net/man/2/nanosleep
    int result = nanosleep(&request, &remaining);
    if( -1 == result )
    {
        result = errno;
    }
#else
    #error missing POSIX nanosleep function
#endif
    if(result == EINTR) {
        // in case of an interrupt return the remaining time
        return Time::fromMicros(remaining.tv_sec * 1000000 + remaining.tv_nsec / 1000);
    } else {
        // otherwise just return zero
        return Time();
    }
}
#endif

const Time Time::ZERO(0);

Time::Time(Time::TimeType micros) : m_micros(micros) {
}

Time::Time() : m_micros(0) {
}

Time::TimeType Time::micros() const {
    return m_micros;
}

Time Time::fromMicros(Time::TimeType micros) {
    return Time(micros);
}

Time Time::fromMillis(Time::TimeType millis) {
    return Time(millis * 1000);
}

Time Time::since(const Time& reference) {
    return Time( now() - reference );
}

Time& Time::operator +=(const Time &t) {
    m_micros += t.micros();
    return *this;
}

Time& Time::operator -=(const Time &t) {
    m_micros -= t.micros();
    return *this;
}

Time Time::operator +(const Time &t) const {
    return Time(micros() + t.micros());
}

Time Time::operator -(const Time &t) const {
    return Time(micros() - t.micros());
}

Time Time::operator * (int scalar) const {
    return Time(micros() * scalar);
}

Time Time::operator / (int scalar) const {
    return Time(micros() / scalar);
}

Time& Time::operator *= (int scalar) {
    m_micros *= scalar;
    return *this;
}

Time& Time::operator /= (int scalar) {
    m_micros /= scalar;
    return *this;
}

bool Time::operator >(const Time &t) const {
    return micros() > t.micros();
}

bool Time::operator >=(const Time &t) const {
    return micros() >= t.micros();
}

bool Time::operator <(const Time &t) const {
    return micros() < t.micros();
}

bool Time::operator <=(const Time &t) const {
    return micros() <= t.micros();
}

bool Time::operator ==(const Time &t) const {
    return micros() == t.micros();
}

bool Time::operator !=(const Time &t) const {
    return micros() != t.micros();
}

std::ostream& operator <<(std::ostream &stream, const Time &t) {
    stream << t.micros() << " us";
    return stream;
}

}  // namespace lms
