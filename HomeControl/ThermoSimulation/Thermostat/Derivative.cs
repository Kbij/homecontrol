using System.Collections.Generic;
using ThermoSimulation.Interfaces;

namespace ThermoSimulation.Thermostat
{
    public class Derivative : DerivativeIf
    {
        private WeigtedAverageFilter mTemperatureAverage;
        private WeigtedAverageFilter mDxDtAverage;
        private WeigtedAverageFilter mDxDt2Average;
        private long mPrevTime = 0;
        private double mPrevTemperature = 0;
        private double mPrevDxDt = 0;
        private double mDxDt = 0;
        private double mDxDt2 = 0;

        public Derivative()
        {
            mTemperatureAverage = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
            mDxDtAverage = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
            mDxDt2Average = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
        }

        public void addTemperature(long currentTime, double currentTemperature)
        {
            double currentAvgTemperature = mTemperatureAverage.AddValue(currentTemperature);
            if (mPrevTime > 0)
            {
                double dxdt = (currentAvgTemperature - mPrevTemperature) / (currentTime - mPrevTime);
                mDxDt = mDxDtAverage.AddValue(dxdt);

                double dxdt2 = (mDxDt - mPrevDxDt) / (currentTime - mPrevTime);
                mDxDt2 = mDxDt2Average.AddValue(dxdt2);

                mPrevDxDt = mDxDt;
            }
            mPrevTemperature = currentAvgTemperature;
            mPrevTime = currentTime;
        }

        public Trend temperatureTrend()
        {
            return Trend.Unknown;
        }

        public Acceleration temperatureAccelaration()
        {
            return Acceleration.Unkown;
        }

        public double firstDerivative()
        {
            return mDxDt;
        }

        public double secondDerivative()
        {
            return mDxDt2;
        }

        public bool isStable()
        {
            return mTemperatureAverage.isStable() && mDxDtAverage.isStable() && mDxDt2Average.isStable();
        }
    }
}
