using ThermoSimulation.Interfaces;

namespace ThermoSimulation.Thermostat
{
    /*
     * No Logic in here, only connecting different parts
     * 
     */

    class PredictiveThermoDataStub : PredictiveThermoDataIf
    {
        public int StartDelaySeconds()
        {
            return 60;
        }

        public int StopDelaySeconds()
        {
            return 60;
        }
    }

    public class PredictiveThermostat : ThermostatIf
    {
        PredictiveThermoDataStub mDataStub;
        PredictiveTwoPoint mPredictiveTwoPoint;
        public PredictiveThermostat(string dbString)
        {
            mDataStub = new PredictiveThermoDataStub();
            mPredictiveTwoPoint = new PredictiveTwoPoint(mDataStub);
        }
        public void registerListener(ThermostatListenerIf listener)
        {
            mPredictiveTwoPoint.registerListener(listener);
        }
        public void unRegisterListener()
        {
            mPredictiveTwoPoint.unRegisterListener();
        }
        public void setPointChanged(long timePoint, double setTemperature)
        {
            mPredictiveTwoPoint.setPointChanged(timePoint, setTemperature);
        }

        public void temperatureChanged(long timePoint, double temperature)
        {
            mPredictiveTwoPoint.setPointChanged(timePoint, temperature);
        }


    }
}
