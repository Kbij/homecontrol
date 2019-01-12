using System;

namespace HomeControl.Comm
{
    public class GpsLocation: ICommObject
    {
        public const byte ObjectID = 10;

        public double Latitude { get; set; }
        public double Longitude { get; set; }
        public DateTime TimeStamp { get; set; }
        public double Accuracy { get; set; }
        public int BatteryLevel { get; set; }

        public GpsLocation()
        {
            TimeStamp = DateTime.Now;
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<GpsLocation>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<GpsLocation>.DeSerialize(json);
            Latitude = value.Latitude;
            Longitude = value.Longitude;
            TimeStamp = value.TimeStamp;
            BatteryLevel = value.BatteryLevel;
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