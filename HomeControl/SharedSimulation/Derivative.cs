using System.Collections.Generic;

namespace SharedSimulation
{
    public class Derivative : DerivativeIf
    {
        RunningAverageFilter mTemperatureAverage = new RunningAverageFilter(10);
        RunningAverageFilter mDxDtAverage = new RunningAverageFilter(10);
        RunningAverageFilter mDxDt2Average = new RunningAverageFilter(10);

        private const decimal DXDT_HYSTERESIS = (decimal) (1.0 / 50000.0);
        private const decimal DXDT2_HYSTERESIS = (decimal) (1.0 / 100000000.0);
        private long mPrevTime = 0;
        private decimal mPrevTemperature = 0;
        private decimal mPrevDxDt = 0;
        private decimal mDxDt = 0;
        private decimal mDxDt2 = 0;

        public Derivative()
        {
            //mTemperatureAverage = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
            //            mDxDtAverage = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
            //            mDxDt2Average = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
            mTemperatureAverage = new RunningAverageFilter(20);
            mDxDtAverage = new RunningAverageFilter(50);
            mDxDt2Average = new RunningAverageFilter(100);
             //mTemperatureAverage = OnlineFilter.CreateLowpass(ImpulseResponse.Finite, 48000, 8000);
          //  mTemperatureAverage = OnlineFilter.CreateDenoise(20);
        }

        public double addTemperature(long currentTime, double currentTemperature)
        {
            //decimal avgTemperature = mTemperatureAverage.AddValue((decimal)currentTemperature);
            decimal avgTemperature = mTemperatureAverage.AddValue((decimal)currentTemperature);

            if (mPrevTime > 0)
            {
                decimal dxdt = ((decimal )avgTemperature - mPrevTemperature) / (currentTime - mPrevTime);
                mDxDt = mDxDtAverage.AddValue(dxdt);

                decimal dxdt2 = (mDxDt - mPrevDxDt) / (currentTime - mPrevTime);
                mDxDt2 = mDxDt2Average.AddValue(dxdt2);

                mPrevDxDt = mDxDt;
            }

            mPrevTemperature = (decimal)avgTemperature;
            mPrevTime = currentTime;
            return (double )avgTemperature;
        }

        public Trend temperatureTrend()
        {
            if (mDxDt > DXDT_HYSTERESIS)
            {
                return Trend.Rising;
            }else if(mDxDt < -DXDT_HYSTERESIS)
            {
                return Trend.Falling;
            }else
            {
                return Trend.Stable;
            }
        }

        public Acceleration temperatureAccelaration()
        {
            if (mDxDt2 > DXDT2_HYSTERESIS)
            {
                return Acceleration.Accelerating;
            }
            else if (mDxDt2 < -DXDT2_HYSTERESIS)
            {
                return Acceleration.Decelerating;
            }
            else
            {
                return Acceleration.Linear;
            }
        }

        public decimal firstDerivative()
        {
            return mDxDt;
        }

        public decimal secondDerivative()
        {
            return mDxDt2;
        }

        public bool isStable()
        {
            return mTemperatureAverage.isStable() && mDxDtAverage.isStable() && mDxDt2Average.isStable();
        }
    }
}
