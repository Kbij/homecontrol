using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor.DAL
{
    class RunningAverageFilter2
    {
        int mK;
        int mSampleCount;
        double mFilterSum;

        public RunningAverageFilter2(int k)
        {
            mK = k;
            mFilterSum = 0;
            mSampleCount = 0;
        }

        public double AddValue(double value)
        {
            mFilterSum = mFilterSum - (mFilterSum / mK) + value;
            double filteredTemperature = mFilterSum / mK;
//            filteredTemperature = round(filteredTemperature * 10) / 10;
            if (mSampleCount > (mK * 5))
            {
                return filteredTemperature;
            }
            else
            {
                ++mSampleCount;
                return value;
            }
        }
    }
}
