using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor.DAL
{
    class ExponentialWeightedAverage
    {
        double mAlpha;
        double mValue;
        bool mStarted;
        public ExponentialWeightedAverage(double alpha)
        {
            mAlpha = alpha;
            mValue = 0;
            mStarted = false;
        }

        public double AddValue(double value)
        {
            mValue = (mAlpha * value) + (1.0 - mAlpha) * mValue;
            if (mStarted)
            {
                return mValue;
            }
            if (Math.Abs(value - mValue) < 0.1)
            {
                //Return the filtered value next time
                mStarted = true;
            }

            return value;
        }
    }
}
