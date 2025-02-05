using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor.DAL
{
    public class GpsClient
    {
        public string Name { get; set; }
        public DateTime LastMessage { get; set; }
        public int LocationInterval { get; set; }
        public int BatteryLevel { get; set; }
        public List<GpsLocation> Locations { get; set; }
        public GpsClient(string name, DateTime lastMessage, int locationInterval, int batteryLevel)
        {
            Name = name;
            LastMessage = lastMessage;
            LocationInterval = locationInterval;
            BatteryLevel = batteryLevel;

            Locations = new List<GpsLocation>();
        }
    }
}
