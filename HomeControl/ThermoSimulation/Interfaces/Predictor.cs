using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Interfaces
{
    public interface Predictor
    {
        void AddTemperature(double temperature);
        double SecondsForTargetTemperature(double target);
        bool TargetInSight(double target);
    }
}
