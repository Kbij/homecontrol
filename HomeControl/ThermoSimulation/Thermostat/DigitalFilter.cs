using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Thermostat
{
    class DigitalFilter
    {
        decimal yn_1 = 0;
        decimal yn_2 = 0;
        decimal xn_1 = 0;
        decimal xn_2 = 0;
        public DigitalFilter()
        {

        }
        public bool isStable()
        {
            return true;
        }

        public decimal AddValue(decimal value)
        {
            decimal result = ((-1 * yn_1 )/ 4 ) + ((3 * yn_2 )/ 8 ) + value + (2 * xn_1) + xn_2;
            yn_2 = yn_1;
            yn_1 = result;

            xn_2 = xn_1;
            xn_1 = value;
            return result;
        }
    }
}
