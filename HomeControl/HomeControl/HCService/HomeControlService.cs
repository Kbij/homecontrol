using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.Util;
using Android.Locations;
using HomeControl.Comm;
using System.IO;

namespace HomeControl.HCService
{
    [Service]
    public class HomeControlService : Service, ILocationListener
    {
        HCServiceBinder mBinder;
        LocationManager _locationManager;
        const string TAG = "HomeControlService";
        string _locationProvider;
        CommModel mCommModel;
        HCLogger mLog;
        Location lastLocation;
        private int TWO_MINUTES = 1000 * 60 * 2;

        public HomeControlService()
        {
           // AppDomain currentDomain = AppDomain.CurrentDomain;
           // currentDomain.UnhandledException += HandleExceptions;

            //mLog = new HCLogger("192.168.10.10", 8001, "Service.log");
            //mLog.SendToHost("HomeControlService", "HCService constructor");
            //mCommModel = new CommModel(mLog);
        }

        static void HandleExceptions(object sender, UnhandledExceptionEventArgs ex)
        {
            string fileName = "/sdcard/Android/data/HomeControl.HomeControl/files/HomeControlUnhandled.log";//Path.Combine(path, "HomeControl.log");

            using (var streamWriter = new StreamWriter(fileName, true))
            {
                streamWriter.WriteLine(string.Format("{0}:{1}", DateTime.Now, ex.ToString()));
            }
        }

        #region lifetime
        public override StartCommandResult OnStartCommand(Android.Content.Intent intent, StartCommandFlags flags, int startId)
        {

            mLog = new HCLogger("192.168.10.10", 8001, "Service.log");
    
            mLog.SendToHost("HomeControlService", "HomeControlService started");
            //mLog.SendToHost("HomeControlService", "HCService constructor");
            mCommModel = new CommModel(mLog);
            mCommModel.startComm();

            InitializeLocationManager();

            return StartCommandResult.Sticky;
        }

        public override void OnCreate()
        {
           // mLog.SendToHost("HomeControlService", "HomeControlService OnCreate");
        }

        public override void OnDestroy()
        {
            ////Service is destroyed; restart it...
            //var intent = new Intent("RestartHCService");
            //SendBroadcast(intent);

            mLog.SendToHost("HomeControlService", "HomeControlService destroyed");
            mCommModel.Dispose();
            base.OnDestroy();
        }

        public override void OnTaskRemoved(Intent rootIntent)
        {
            mLog.SendToHost("HomeControlService", "HomeControlService ontaskremoved");
            // TODO Auto-generated method stub
            //Intent restartService = new Intent(getApplicationContext(), this.getClass());
            //restartService.setPackage(getPackageName());
            //PendingIntent restartServicePI = PendingIntent.getService(getApplicationContext(), 1, restartService, PendingIntent.FLAG_ONE_SHOT);
            //AlarmManager alarmService = (AlarmManager)getApplicationContext().getSystemService(Context.ALARM_SERVICE);
            //alarmService.set(AlarmManager.ELAPSED_REALTIME, SystemClock.elapsedRealtime() + 1000, restartServicePI);
        }

        public override IBinder OnBind(Intent intent)
        {
            mBinder = new HCServiceBinder(this);
            return mBinder;
        }
        #endregion lifetime

        #region hcservice
        public void registerCommReceiver(ICommReceiver receiver)
        {
            mCommModel.registerCommReceiver(receiver);
        }

        public void unRegisterCommReceiver(ICommReceiver receiver)
        {
            mCommModel.unRegisterCommReceiver(receiver);
        }

        public void sendObject(ICommObject obj)
        {
            if (mCommModel != null)
            {
                mCommModel.sendObjectQueued(obj);
            }
        }

        void InitializeLocationManager()
        {
            _locationManager = (LocationManager)GetSystemService(LocationService);
            Criteria criteriaForLocationService = new Criteria{Accuracy = Accuracy.Medium, PowerRequirement = Power.Low};
            IList<string> acceptableLocationProviders = _locationManager.GetProviders(criteriaForLocationService, true);

            foreach (var provider in acceptableLocationProviders)
            {
                mLog.SendToHost(TAG, "Provider: " + provider);
            }

            if (acceptableLocationProviders.Any())
            {
                _locationProvider = acceptableLocationProviders.First();
            }
            else
            {
                _locationProvider = string.Empty;
            }
            mLog.SendToHost("HomeControlService", string.Format("Location provider used: {0}", _locationProvider));
            _locationManager.RequestLocationUpdates(_locationProvider, 1 * 60 * 1000, 0, this); // Min 1 minute
            //_locationManager.RequestLocationUpdates(_locationProvider, 0, 0, this); // Min 1 minute

            MessageObject msg = new MessageObject();
            msg.Message = string.Format("Using location provider: {0}", _locationProvider);
            mCommModel.sendObjectQueued(msg);
        }

        public void OnLocationChanged(Location location)
        {
            mLog.SendToHost(TAG, string.Format("Location: {0}", location.ToString()));
            //_currentLocation = location;
            if (location == null)
            {
                mLog.SendToHost(TAG, "Unable to determine your location. Try again in a short while.");
            }
            else
            {
                if (location.Accuracy < 500)
                {
                    Log.Debug(TAG, string.Format("lat: {0:f6}, lon: {1:f6}, acc:{2}", location.Latitude, location.Longitude, location.Accuracy));
                    if (isBetterLocation(location, lastLocation))
                    {
                        GpsLocation loc = new GpsLocation { Latitude = location.Latitude, Longitude = location.Longitude, Accuracy = location.Accuracy };
                        mCommModel.sendObjectQueued(loc);
                    }
                    else
                    {
                        mLog.SendToHost(TAG, "Received location is worse than previous");
                    }
                }

                lastLocation = location;
            }
        }

        public void OnProviderDisabled(string provider)
        {
            mLog.SendToHost(TAG, "Provider disabled");
            MessageObject msg = new MessageObject();
            msg.Message = "Provider disabled";
            mCommModel.sendObjectQueued(msg);
        }

        public void OnProviderEnabled(string provider)
        {
            mLog.SendToHost(TAG, "Provider enabled");
            MessageObject msg = new MessageObject();
            msg.Message = "Provider enabled";
            mCommModel.sendObjectQueued(msg);
        }

        public void OnStatusChanged(string provider, Availability status, Bundle extras)
        {
            MessageObject msg = new MessageObject();
            msg.Message = string.Format("Provider status changed, provider: {0}, status: {1}", provider, status.ToString());
            mCommModel.sendObjectQueued(msg);
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
            bool isSignificantlyNewer = timeDelta > TWO_MINUTES;
            bool isSignificantlyOlder = timeDelta < -TWO_MINUTES;
            bool isNewer = timeDelta > 0;

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
        #endregion hcservice
    }

}