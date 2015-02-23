#ifndef KALMAN_MEASUREMENTMODEL_H_
#define KALMAN_MEASUREMENTMODEL_H_

#include "Matrix.h"

namespace Kalman {
    template<typename T, class StateType, class MeasurementType>
    class MeasurementModel
    {
        static_assert( StateType::length       > 0, "State vector must contain at least 1 element");
        static_assert( MeasurementType::length > 0, "Measurement vector must contain at least 1 element");
    public:
        //! Type of the state vector
        typedef StateType State;
        //! Type of the measurement vector
        typedef MeasurementType Measurement;
        //! Type of the measurement covariance matrix
        typedef Matrix<T, Measurement::length, Measurement::length> Covariance;
        //! Kalman gain matrix type
        typedef Matrix<T, State::length, Measurement::length> KalmanGain;
        
    protected:
        //! Measurement noise covariance
        Covariance R;
        
    public:
        /**
         * Measurement Model Function h
         * 
         * Predicts the estimated measurement value given the current state estimate x
         */
        virtual Measurement h(const State& x) const = 0;
        
        const Covariance& getCovariance() const
        {
            return R;
        }
    protected:
        MeasurementModel()
        {
            R.setIdentity();
        }
        ~MeasurementModel() {}
    };
}

#endif
