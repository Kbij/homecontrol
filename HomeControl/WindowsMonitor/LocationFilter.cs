using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WindowsMonitor.DAL;

namespace WindowsMonitor
{
    public class LocationFilter
    {
        private int mMinDistanceMeters;
        private int mMaxMinutes;
        public LocationFilter(int minDistanceMeters, int maxMinutes)
        {
            mMinDistanceMeters = minDistanceMeters;
            mMaxMinutes = maxMinutes;
        }

        public List<Int32> filterLocations(List<GpsLocation> locations)
        {
            List<int> result = new List<int>();
            GpsLocation previous = null;
            foreach (var location in locations)
            {
                if (previous != null)
                {
                    double distance = location.distance(previous);
                    double totalMinutes = (location.TimeStamp - previous.TimeStamp).TotalMinutes;
                    //If distance is not big enough. But always keep the last element in the list 
                    if (distance < mMinDistanceMeters && //If to close
                        (!(location.Equals(locations.Last())) && //And not the last in the list
                        (totalMinutes < mMaxMinutes))) //and not older than mMaxMinutes
                    {
                        result.Add(location.Id);
                    }
                    else
                    {
                        previous = location;
                    }
                }
                else
                {
                    previous = location;
                }
            }
            return result;
        }
    }
}
