#ifndef KALMAN_EXTENDEDKALMANFILTER_H_
#define KALMAN_EXTENDEDKALMANFILTER_H_

#include "KalmanFilterBase.h"
#include "LinearizedSystemModel.h"
#include "LinearizedMeasurementModel.h"

namespace Kalman {
    
    /**
     * Extended Kalman Filter base class
     *
     * Concrete EKF implementations should derive from this base class
     */
    template<typename T, class StateType>
    class ExtendedKalmanFilter : public KalmanFilterBase<T, StateType>
    {
    public:
        typedef KalmanFilterBase<T, StateType> Base;
        
        using typename Base::State;
                
        template<class Measurement>
        using MeasurementModelType = LinearizedMeasurementModel<T, State, Measurement>;
        
        template<class Control>
        using SystemModelType = LinearizedSystemModel<T, State, Control>;
        
    protected:
        
        using Base::x;
        using Base::P;
        
    public:
        template<class Control>
        const State& predict( SystemModelType<Control>& s  )
        {
            // predict state (without control)
            Control u;
            u.setZero();
            return predict( s, u );
        }
        
        template<class Control>
        const State& predict(SystemModelType<Control>& s, const Control& u)
        {
            // predict state
            x = s.f(x, u);
            
            // predict covariance
            P  = ( s.F * P * s.F.transpose() ) + ( s.W * s.getCovariance() * s.W.transpose() );
            
            // return state prediction
            return this->getState();
        }
        
        template<class Measurement>
        const State& update( MeasurementModelType<Measurement>& m, const Measurement& z )
        {
            m.updateJacobians( x );
            
            // COMPUTE KALMAN GAIN
            // compute innovation covariance
            typename MeasurementModelType<Measurement>::Covariance S = ( m.H * P * m.H.transpose() ) + ( m.V * m.getCovariance() * m.V.transpose() );
            
            // compute kalman gain
            typename MeasurementModelType<Measurement>::KalmanGain K = P * m.H.transpose() * S.inverse();
            
            // UPDATE STATE ESTIMATE AND COVARIANCE
            // Update state using computed kalman gain and innovation
            x += K * ( z - m.h( x ) );
            
            // Update covariance
            P -= K * m.H * P;
            
            // return updated state estimate
            return this->getState();
        }
        
    protected:
        ExtendedKalmanFilter()
        {
            // Setup state and covariance
            x.setZero();
            P.setIdentity();
        }
    };
}

#endif
