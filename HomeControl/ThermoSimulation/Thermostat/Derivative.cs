using System.Collections.Generic;
using ThermoSimulation.Interfaces;
using MathNet.Filtering;

namespace ThermoSimulation.Thermostat
{
    public class Derivative : DerivativeIf
    {
        // private WeigtedAverageFilter mTemperatureAverage;
        // private WeigtedAverageFilter mDxDtAverage;
        // private WeigtedAverageFilter mDxDt2Average;
        RunningAverageFilter mTemperatureAverage = new RunningAverageFilter(50);
        RunningAverageFilter mDxDtAverage = new RunningAverageFilter(50);
        RunningAverageFilter mDxDt2Average = new RunningAverageFilter(50);

       // DigitalFilter mTemperatureAverage = new DigitalFilter();
        //DigitalFilter mDxDtAverage = new DigitalFilter();
        //DigitalFilter mDxDt2Average = new DigitalFilter();
        //OnlineFilter mTemperatureAverage;

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
            return Trend.Unknown;
        }

        public Acceleration temperatureAccelaration()
        {
            return Acceleration.Unkown;
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
            return /*mTemperatureAverage.isStable() && */mDxDtAverage.isStable() && mDxDt2Average.isStable();
        }
    }
}
