using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharedSimulation
{
    public enum Trend { Unknown, Rising, Stable, Falling };
    public enum Acceleration { Unkown, Accelerating, Linear, Decelerating }

    public interface DerivativeIf
    {
        double addTemperature(long time, double currentTemperature);
        bool isStable();

        Trend temperatureTrend();
        Acceleration temperatureAccelaration();

        decimal firstDerivative();
        decimal secondDerivative();
    }
}
