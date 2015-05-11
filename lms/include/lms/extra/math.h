namespace lms{
namespace math{
template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
}
}
