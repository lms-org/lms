#ifndef KALMAN_MATRIX_H_
#define KALMAN_MATRIX_H_

#include <Eigen/Dense>

#define KALMAN_VECTOR(NAME, T, N)                                                       \
    static constexpr int length = N;                                                    \
    typedef Kalman::Vector<T, N> Base;                                                  \
                                                                                        \
    NAME(void) : Kalman::Vector<T, N>() {}                                              \
                                                                                        \
    template<typename OtherDerived>                                                     \
    NAME(const Eigen::MatrixBase<OtherDerived>& other) : Kalman::Vector<T, N>(other) {} \
                                                                                        \
    template<typename OtherDerived>                                                     \
    NAME& operator= (const Eigen::MatrixBase <OtherDerived>& other)                     \
    {                                                                                   \
        this->Base::operator=(other);                                                   \
        return *this;                                                                   \
    }

namespace Kalman {
    template<typename T, int rows, int cols>
    using Matrix = Eigen::Matrix<T, rows, cols>;
    
    template<typename T, int N>
    class Vector : public Matrix<T, N, 1>
    {
    public:
        static constexpr int length = N;
        typedef Matrix<T, N, 1> Base;
        
        Vector(void) : Matrix<T, N, 1>() {}
        
        template<typename OtherDerived>
        Vector(const Eigen::MatrixBase<OtherDerived>& other)
            : Matrix<T, N, 1>(other)
        { }
        template<typename OtherDerived>
        Vector& operator= (const Eigen::MatrixBase <OtherDerived>& other)
        {
            this->Base::operator=(other);
            return *this;
        }
    };
}

#endif
