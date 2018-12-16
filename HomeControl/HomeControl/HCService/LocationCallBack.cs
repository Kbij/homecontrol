using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.Gms.Location;
using Android.Locations;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using HomeControl.Comm;

namespace HomeControl.HCService
{
    public class LocationCallBack : LocationCallback
    {
        private HCLogger mLog;
        string mLocationLogFileName;
        Location mLastLocation;
        ILocationListener mListener;
        private int MAXIMUM_LOCATION_AGE = 1000 * 60 * 2;

        public LocationCallBack(HCLogger log, ILocationListener listener)
        {
            mLog = log;
            string path = Android.OS.Environment.ExternalStorageDirectory.AbsolutePath;
            mLocationLogFileName = Path.Combine(path, "locationlog.csv");
            mListener = listener;
        }

        public override void OnLocationAvailability(LocationAvailability locationAvailability)
        {
            mLog.SendToHost("LocationCallBack", string.Format("IsLocationAvailable: {0}", locationAvailability.IsLocationAvailable));
        }

        public override void OnLocationResult(LocationResult result)
        {
            if (result.Locations.Any())
            {
                var location = result.Locations.First();
                mLog.SendToHost("HomeControlService", string.Format("lat: {0:f6}, lon: {1:f6}, acc:{2}, time:{3}, provider:{4}", location.Latitude, location.Longitude, location.Accuracy, location.Time, location.Provider));
                try
                {
                    using (var streamWriter = new StreamWriter(mLocationLogFileName, true))
                    {
                        streamWriter.WriteLine(string.Format("{0};{1};{2};{3};{4}", location.Latitude, location.Longitude, location.Accuracy, location.Time, location.Provider));
                    }
                }
                catch (Exception )
                {
                }

                if (location.Accuracy < 500)
                {
                    if (isBetterLocation(location, mLastLocation))
                    {
                        mLog.SendToHost("HomeControlService", "is a better location");
                        mLastLocation = location;
                        if (mListener != null) mListener.NewLocation(location);
                    }
                    else
                    {
                        mLog.SendToHost("LocationCallBack", "Received location is worse than previous");
                    }
                }
            }
            else
            {
                // No locations to work with.
            }
        }

        /** Determines whether one Location reading is better than the current Location fix
        * @param location  The new Location that you want to evaluate
        * @param currentBestLocation  The current Location fix, to which you want to compare the new one
        */
        protected bool isBetterLocation(Location location, Location currentBestLocation)
        {
            if (currentBestLocation == null)
            {
                // A new location is always better than no location
                return true;
            }

            // Check whether the new location fix is newer or older
            long timeDelta = location.Time - currentBestLocation.Time;
            bool isSignificantlyNewer = timeDelta > MAXIMUM_LOCATION_AGE;
            bool isSignificantlyOlder = timeDelta < -MAXIMUM_LOCATION_AGE;
            bool isNewer = timeDelta > 0;
            double distanceBetween = distance(location, currentBestLocation);
            double accuracySum = location.Accuracy + currentBestLocation.Accuracy;
            mLog.SendToHost("HomeControlService", string.Format("timeDelta: {0}", timeDelta));
            mLog.SendToHost("HomeControlService", string.Format("isSignificantlyNewer: {0}", isSignificantlyNewer));
            mLog.SendToHost("HomeControlService", string.Format("isSignificantlyOlder: {0}", isSignificantlyOlder));
            mLog.SendToHost("HomeControlService", string.Format("isNewer: {0}", isNewer));
            mLog.SendToHost("HomeControlService", string.Format("distanceBetween: {0}", distanceBetween));
            mLog.SendToHost("HomeControlService", string.Format("accuracySum: {0}", accuracySum));


            // If the distance between both is larger than the sum of the accuracy
            if (distanceBetween > accuracySum)
            {
                return true;
            }
            // If it's been more than two minutes since the current location, use the new location
            // because the user has likely moved
            if (isSignificantlyNewer)
            {
                return true;
                // If the new location is more than two minutes older, it must be worse
            }
            else if (isSignificantlyOlder)
            {
                return false;
            }

            // Check whether the new location fix is more or less accurate
            int accuracyDelta = (int)(location.Accuracy - currentBestLocation.Accuracy);
            bool isLessAccurate = accuracyDelta > 0;
            bool isMoreAccurate = accuracyDelta < 0;
            bool isSignificantlyLessAccurate = accuracyDelta > 200;

            // Check if the old and new location are from the same provider
            bool isFromSameProvider = isSameProvider(location.Provider,
                    currentBestLocation.Provider);

            // Determine location quality using a combination of timeliness and accuracy
            if (isMoreAccurate)
            {
                return true;
            }
            else if (isNewer && !isLessAccurate)
            {
                return true;
            }
            else if (isNewer && !isSignificantlyLessAccurate && isFromSameProvider)
            {
                return true;
            }
            return false;
        }

        /** Checks whether two providers are the same */
        private bool isSameProvider(String provider1, String provider2)
        {
            if (provider1 == null)
            {
                return provider2 == null;
            }
            return provider1.Equals(provider2);
        }

        public double distance(Location first, Location second)
        {
            double theta = first.Longitude - second.Longitude;
            double dist = Math.Sin(deg2rad(first.Latitude)) * Math.Sin(deg2rad(second.Latitude)) + Math.Cos(deg2rad(first.Latitude)) * Math.Cos(deg2rad(second.Latitude)) * Math.Cos(deg2rad(theta));
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