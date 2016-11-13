using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor.DAL
{
    class RunningAverageFilter
    {
        private int mNumberOfPoints;
        private List<double> mSum;

        public RunningAverageFilter(int points)
        {
            mNumberOfPoints = points;
            mSum = new List<double>();
        }

        public double AddValue(double value)
        {
            mSum.Add(value);
            while (mSum.Count > mNumberOfPoints)
            {
                mSum.RemoveAt(0);
            }
            if (mSum.Count == mNumberOfPoints)
            {
                return mSum.Sum() / mNumberOfPoints;
            }
            else
            {
                return value;
            }
        }
    }
}
