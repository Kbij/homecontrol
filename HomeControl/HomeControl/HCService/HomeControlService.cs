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
using Android.Gms.Common;
using Android.Gms.Location;

namespace HomeControl.HCService
{
    [Service]
    public class HomeControlService : Service, ILocationListener //, LocationCallback //Android.Gms.Location.ILocationListener
    {
        HCServiceBinder mBinder;
        const string TAG = "HomeControlService";
        CommModel mCommModel;
        HCLogger mLog;
        FusedLocationProviderClient mFusedLocationProviderClient;
        LocationCallBack mLocationCallBack;

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
            try
            {
                string path = Android.OS.Environment.ExternalStorageDirectory.AbsolutePath;
                string fileName = Path.Combine(path, "HomeControlUnhandled.log");

                using (var streamWriter = new StreamWriter(fileName, true))
                {
                    streamWriter.WriteLine(string.Format("{0}:{1}", DateTime.Now, ex.ToString()));
                }
            }
            catch (Exception)
            { }
        }

        #region lifetime
        public override StartCommandResult OnStartCommand(Android.Content.Intent intent, StartCommandFlags flags, int startId)
        {
            mLog = new HCLogger("192.168.10.10", 8001, "Service.log");
    
            mLog.SendToHost("HomeControlService", "HomeControlService started");
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
            mFusedLocationProviderClient = LocationServices.GetFusedLocationProviderClient(this);
            mLocationCallBack = new LocationCallBack(mLog, this);

            Android.Gms.Location.LocationRequest locationRequest = new LocationRequest()
                                  .SetPriority(LocationRequest.PriorityHighAccuracy)
                                  .SetInterval(60 * 1000 * 3)
                                  .SetFastestInterval(60 * 1000 * 1);
            mFusedLocationProviderClient.RequestLocationUpdates(locationRequest, mLocationCallBack, null);
        }

        public void NewLocation(Location location)
        {
            if (location != null)
            {
                mLog.SendToHost("HomeControlService", "Sending location ....");
                GpsLocation loc = new GpsLocation { Latitude = location.Latitude, Longitude = location.Longitude, Accuracy = location.Accuracy };
                mCommModel.sendObjectQueued(loc);
            }
        }

        public void OnStatusChanged(string provider, Availability status, Bundle extras)
        {
            MessageObject msg = new MessageObject();
            msg.Message = string.Format("Provider status changed, provider: {0}, status: {1}", provider, status.ToString());
            mCommModel.sendObjectQueued(msg);
        }
        #endregion hcservice
    }

}