#ifndef LMS_EXTRA_MATH_MONITOR_H
#define LMS_EXTRA_MATH_MONITOR_H
namespace lms{
namespace math{
template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
}
}

//TODO implement fmod to limit angles

//TODO get sgn from angles for given sin cos etc.
#endif
