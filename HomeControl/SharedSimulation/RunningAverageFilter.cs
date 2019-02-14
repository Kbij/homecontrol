using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharedSimulation
{
    class RunningAverageFilter
    {
        private int mNumberOfPoints;
        private List<decimal> mSum;

        public RunningAverageFilter(int points)
        {
            mNumberOfPoints = points;
            mSum = new List<decimal>();
        }

        public bool isStable()
        {
            return (mSum.Count >= mNumberOfPoints);
        }

        public decimal AddValue(decimal value)
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
