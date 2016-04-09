using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor.DAL
{
    public class GpsLocation
    {
        public double Latitude { get; set; }
        public double Longitude { get; set; }
        public double Accuracy { get; set; }
        public DateTime TimeStamp { get; set; }

        public GpsLocation(double latitude, double longitude, double accuracy, DateTime timeStamp)
        {
            Latitude = latitude;
            Longitude = longitude;
            Accuracy = accuracy;
            TimeStamp = timeStamp;
        }

        public string toString()
        {
            return string.Format("Lat: {0}, Lon: {1}, Time: {2}, Accuracy: {3}", Latitude, Longitude, TimeStamp, Accuracy);
        }

        public string url()
        {
            return string.Format("http://maps.google.com/maps?z=12&t=m&q=loc:{0}+{1}", Latitude.ToString(CultureInfo.InvariantCulture), Longitude.ToString(CultureInfo.InvariantCulture));
        }

        public double distance(GpsLocation other)
        {
            double theta = Longitude - other.Longitude;
            double dist = Math.Sin(deg2rad(Latitude)) * Math.Sin(deg2rad(other.Latitude)) + Math.Cos(deg2rad(Latitude)) * Math.Cos(deg2rad(other.Latitude)) * Math.Cos(deg2rad(theta));
            dist = Math.Acos(dist);
            dist = rad2deg(dist);
            dist = dist * 60 * 1.1515;
            dist = dist * 1609.344;

            return (dist);
        }

        //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        //::  This function converts decimal degrees to radians             :::
        //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        private double deg2rad(double deg)
        {
            return (deg * Math.PI / 180.0);
        }

        //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        //::  This function converts radians to decimal degrees             :::
        //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        private double rad2deg(double rad)
        {
            return (rad / Math.PI * 180.0);
        }
    }
}
