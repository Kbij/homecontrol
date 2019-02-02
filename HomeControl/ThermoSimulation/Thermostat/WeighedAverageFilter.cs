using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThermoSimulation.Thermostat
{
    class WeigtedAverageFilter
    {
        private List<double> mValues;
        private List<double> mFactors;

        public WeigtedAverageFilter(List<double> factors)
        {
            mFactors = factors;
            mValues = new List<double>();
            if (mFactors.Sum() > 1) throw new Exception("Sum of factors should not be bigger than 1");
        }

        public bool isStable()
        {
            return (mValues.Count >= mFactors.Count);
        }

        public double AddValue(double value)
        {
            mValues.Add(value);
            while (mValues.Count > mFactors.Count)
            {
                mValues.RemoveAt(0);
            }
            if (mValues.Count == mFactors.Count)
            {
                double mSum = 0;
                for(int i = 0; i < mValues.Count; ++i)
                {
                    mSum += mValues[i] * mFactors[i];
                }
                return mSum;
            }
            else
            {
                return value;
            }
        }
    }
}
