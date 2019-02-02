using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Interfaces
{
    public enum Trend { Unknown, Rising, Stable, Falling };
    public enum Acceleration { Unkown, Accelerating, Decelerating }

    public interface DerivativeIf
    {
        void addTemperature(long time, double currentTemperature);
        bool isStable();

        Trend temperatureTrend();
        Acceleration temperatureAccelaration();

        double firstDerivative();
        double secondDerivative();
    }
}
