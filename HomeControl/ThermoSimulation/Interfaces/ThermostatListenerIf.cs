using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Interfaces
{
    public interface ThermostatListenerIf
    {
        void heaterOn();
	    void heaterOff();
    }
}
