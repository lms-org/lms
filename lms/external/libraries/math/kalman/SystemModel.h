#ifndef KALMAN_SYSTEMMODEL_H_
#define KALMAN_SYSTEMMODEL_H_

#include "Matrix.h"

namespace Kalman {
    template<typename T, class StateType, class ControlType = Vector<T, 0> >
    class SystemModel
    {
        static_assert( StateType::length   >  0, "State vector must contain at least 1 element");
        static_assert( ControlType::length >= 0, "Control vector must contain at least 0 elements");
    public:
        typedef StateType State;
        typedef ControlType Control;
        
        //! Type of the measurement covariance matrix
        typedef Matrix<T, State::length, State::length> Covariance;
        
    protected:
        //! System noise covariance
        Covariance Q;
        
    public:
        /**
         * Control Model Function h
         * 
         * Predicts the estimated measurement value given the current state estimate x
         */
        virtual State f(const State& x, const Control& u) const = 0;
        
        const Covariance& getCovariance() const
        {
            return Q;
        }
    protected:
        SystemModel()
        {
            Q.setIdentity();
        }
        ~SystemModel() {}
    };
}

#endif
