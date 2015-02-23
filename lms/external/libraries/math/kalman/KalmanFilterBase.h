#ifndef KALMAN_KALMANFILTERBASE_H_
#define KALMAN_KALMANFILTERBASE_H_

#include "Matrix.h"

namespace Kalman {
    
    /**
     * Abstract base class for all Kalman Filters
     * 
     * Template Arguments:
     * * T: numeric type (usually float or double)
     * * StateSize: Number of elements of the state vector (> 0)
     */
    template<typename T, class StateType>
    class KalmanFilterBase
    {
    public:
        static_assert( StateType::length >  0, "State vector must contain at least 1 element");

        //! Type of the state vector
        typedef StateType State;
        //! Type of the state vector
        typedef Matrix<T, State::length, State::length> StateCovariance;
    protected:
        //! Estimated state
        State x;
        
        //! State covariance
        StateCovariance P;
        
    public:
        /**
         * Get current state estimate
         */
        const State& getState() const
        {
            return x;
        }
        
        /**
         * Get current estimate of state-covariance 
         */
        const StateCovariance& getStateCovariance() const
        {
            return P;
        }
        
        /**
         * Initialise state and state covariance
         */
        void init(const State& initialState, const StateCovariance& initialStateCovariance)
        {
            x = initialState;
            P = initialStateCovariance;
        }
    protected:
        KalmanFilterBase()
        {
        }
        
    };
}

#endif
