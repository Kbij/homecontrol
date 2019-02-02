using System;
using ThermoSimulation.Interfaces;

namespace ThermoSimulation.Thermostat
{
    public class PredictiveTwoPoint: ThermostatIf
    {
        private PredictiveThermoDataIf mData;
        public PredictiveTwoPoint(PredictiveThermoDataIf data)
        {
            mData = data;
        }

        public void registerListener(ThermostatListenerIf listener)
        {
            throw new NotImplementedException();
        }

        public void setPointChanged(long timePoint, double setTemperature)
        {
            throw new NotImplementedException();
        }

        public void temperatureChanged(long timePoint, double temperature)
        {
            throw new NotImplementedException();
        }

        public void unRegisterListener()
        {
            throw new NotImplementedException();
        }
    }
}
