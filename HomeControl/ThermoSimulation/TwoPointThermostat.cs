using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation
{
    public class TwoPointThermostat: ThermostatIf
    {
        double mHysteresis;
        ThermostatListenerIf mListener;
        double mSetPoint;
        bool mHeaterOn;
        public TwoPointThermostat(double hysteresis)
        {
            mHysteresis = hysteresis;
        }
        public void registerListener(ThermostatListenerIf listener)
        {
            mListener = listener;
        }
        public void unRegisterListener()
        {
            mListener = null;
        }
        public void temperatureChanged(long timePoint, double temperature)
        {
            double hysteresisTop = mSetPoint + (mHysteresis / 2.0);
            double hysteresisBotom = mSetPoint - (mHysteresis / 2.0);
            if (temperature > hysteresisTop)
            {
                mHeaterOn = false;
            }
            if (temperature < hysteresisBotom)
            {
                mHeaterOn = true;
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
        public void setPointChanged(long timePoint, double setTemperature)
        {
            mSetPoint = setTemperature;
        }

    }
}
