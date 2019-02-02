using System;
using System.Diagnostics;
using ThermoSimulation.Interfaces;

namespace ThermoSimulation.Thermostat
{
    public sealed class PidThermostat : ThermostatIf
    {
        ThermostatListenerIf mListener;
        bool mHeaterOn;
        PidController mPidController;
        long mPreviousTime;
        bool mFirstTempProcessed;
        int mHeaterPeriodSeconds;
        int mMinOnTimeSeconds;
        long mPeriodStartSeconds;
        public double HeaterOutput { get; set; }
        public PidThermostat(int heaterPeriodSeconds, int minOnTimeSeconds)
        {
            mPidController = new PidController(0.8, 0.8, 0, 100, 0);
            mPidController.SetPoint = 16;
            mHeaterOn = false;
            mFirstTempProcessed = false;
            mPreviousTime = 0;
            mHeaterPeriodSeconds = heaterPeriodSeconds;
            mMinOnTimeSeconds = minOnTimeSeconds;
        }

        public void registerListener(ThermostatListenerIf listener)
        {
            mListener = listener;
        }

        public void unRegisterListener()
        {
            mListener = null;
        }

        public void setPointChanged(long timePoint, double setTemperature)
        {
            mPidController.SetPoint = setTemperature;
        }

        public void temperatureChanged(long timePoint, double temperature)
        {
            if (mFirstTempProcessed)
            {
                TimeSpan timeSpan = TimeSpan.FromSeconds(timePoint - mPreviousTime);
                mPidController.ProcessVariable = temperature;
                HeaterOutput = mPidController.ControlVariable(timeSpan);
                Debug.WriteLine(string.Format("Output: {0}", HeaterOutput));
                //If the output is higher than the minimum on time
                if (HeaterOutput > ((mMinOnTimeSeconds / mHeaterPeriodSeconds) * 100))
                {
                    //Heater is not on; switch it on
                    if (!mHeaterOn)
                    {
                        if ((timePoint - mPeriodStartSeconds) > mHeaterPeriodSeconds)
                        {
                            mPeriodStartSeconds = timePoint;
                            mHeaterOn = true;
                        }
                    }
                    else
                    { //The heater is on, let's check if we need to switch it off
                        if (HeaterOutput < ((timePoint - mPeriodStartSeconds) / mHeaterPeriodSeconds) * 100)
                        {
                            mHeaterOn = false;
                        }
                        else
                        {
                            mHeaterOn = true;
                        }
                    }
                }
                else
                {
                    mHeaterOn = false;
                }

                if (mHeaterOn)
                {
                    if (mListener != null) mListener.heaterOn();
                }
                else
                {
                    if (mListener != null) mListener.heaterOff();
                }
            }
            mFirstTempProcessed = true;
            mPreviousTime = timePoint;
        }
    }
}
