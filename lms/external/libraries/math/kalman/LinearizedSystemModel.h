#ifndef KALMAN_LINEARIZEDSYSTEMMODEL_H_
#define KALMAN_LINEARIZEDSYSTEMMODEL_H_

#include "SystemModel.h"

namespace Kalman {
    template<typename T, class StateType>
    class ExtendedKalmanFilter;
    
    template<typename T, class StateType, class ControlType = Vector<T, 0> >
    class LinearizedSystemModel : public SystemModel<T, StateType, ControlType>
    {
        friend class ExtendedKalmanFilter<T, StateType>;
    public:
        typedef SystemModel<T, StateType, ControlType> Base;
        
        using typename Base::State;
        using typename Base::Control;
        using typename Base::Covariance;
        
        //! Measurement model jacobian
        typedef Matrix<T, State::length, State::length> Jacobian;
        
    protected:
        //! System model jacobian
        Jacobian F;
        //! System model noise jacobian
        Jacobian W;
        
        /**
         * Callback function for state-dependent update of Jacobi-matrices F and W before each update step
         */
        virtual void updateSystemJacobians( const State& x )
        {
            // No update by default
        }
    protected:
        LinearizedSystemModel()
        {
            F.setIdentity();
            W.setIdentity();
        }
        ~LinearizedSystemModel() {}
    };
}

#endif
