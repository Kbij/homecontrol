using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Interfaces
{
    public interface ThermostatIf
    {
        void registerListener(ThermostatListenerIf listener);
	    void unRegisterListener();

	    void temperatureChanged(long timePoint, double temperature);
	    void setPointChanged(long timePoint, double setTemperature);
    }
}
