using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Thermostat
{
    class WeigtedAverageFilter
    {
        private List<decimal> mValues;
        private List<decimal> mFactors;

        public WeigtedAverageFilter(List<decimal> factors)
        {
            mFactors = factors;
            mValues = new List<decimal>();
            if (mFactors.Sum() > 1) throw new Exception("Sum of factors should not be bigger than 1");
        }

        public bool isStable()
        {
            return (mValues.Count >= mFactors.Count);
        }

        public double AddValue(double value)
        {
            mValues.Add((decimal)value);
            while (mValues.Count > mFactors.Count)
            {
                mValues.RemoveAt(0);
            }
            if (mValues.Count == mFactors.Count)
            {
                decimal mSum = 0;
                for(int i = 0; i < mValues.Count; ++i)
                {
                    mSum += mValues[i] * mFactors[i];
                }
                return (double) mSum;
            }
            else
            {
                return value;
            }
        }
    }
}
