#ifndef KALMAN_LINEARIZEDMEASUREMENTMODEL_H_
#define KALMAN_LINEARIZEDMEASUREMENTMODEL_H_

#include "MeasurementModel.h"

namespace Kalman {
    template<typename T, class StateType>
    class ExtendedKalmanFilter;
    
    template<typename T, class StateType, class MeasurementType>
    class LinearizedMeasurementModel : public MeasurementModel<T, StateType, MeasurementType>
    {
        friend class ExtendedKalmanFilter<T, StateType>;
    public:
        typedef MeasurementModel<T, StateType, MeasurementType> Base;
        
        using typename Base::State;
        using typename Base::Measurement;
        using typename Base::Covariance;
        
        //! Measurement model jacobian
        typedef Matrix<T, Measurement::length, State::length> Jacobian;
        
    protected:
        //! Measurement model jacobian
        Jacobian H;
        //! Measurement model noise jacobian
        Jacobian V;
        
        /**
         * Callback function for state-dependent update of Jacobi-matrices H and V before each update step
         */
        virtual void updateMeasurementJacobians( const State& x )
        {
            // No update by default
        }
    protected:
        LinearizedMeasurementModel()
        {
            H.setIdentity();
            V.setIdentity();
        }
        ~LinearizedMeasurementModel() {}
    };
}

#endif
