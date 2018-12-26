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
using Android.Gms.Tasks;
using Android.Gms.Common.Apis;
using Java.Lang;

namespace HomeControl.HCService
{
    //https://stackoverflow.com/questions/41294930/android-app-add-geofences-and-receive-intents-in-same-service/41354293
    [Service]
    public class HomeControlService : Service, ICommReceiver, ILocationListener, GoogleApiClient.IConnectionCallbacks, GoogleApiClient.IOnConnectionFailedListener, Android.Gms.Common.Apis.IResultCallback
    {
        HCServiceBinder mBinder;
        const string TAG = "HomeControlService";
        CommModel mCommModel;
        HCLogger mLog;
        FusedLocationProviderClient mFusedLocationProviderClient;
        LocationCallBack mLocationCallBack;
       // GeofencingClient mGeofencingClient;
        private IList<IGeofence> mGeofenceList;
        PendingIntent mPendingIntent;
        GoogleApiClient mGoogleApiClient;
        public const int SERVICE_RUNNING_NOTIFICATION_ID = 10000;
        private const int LOW_UPDATE_FREQ_SECONDS = 4 * 60;
        private const int HIGH_UPDATE_FREQ_SECONDS = 30;
        private const int NOT_MOVING_METERS = 50;
        private const int NOT_MOVING_SECONDS = 5 * 60; // 5 min
        private const string GEOFENCEID = "HomecontrolGeofence";
        private int mCurrentUpdateFreqSeconds;
        private int mRequestedInterval;
        DateTime? mNotMovingStartTime;
        public HomeControlService()
        {
            mPendingIntent = null;
            mGeofenceList = new List<IGeofence>();
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
            catch (System.Exception)
            { }
        }

        #region lifetime
        public override StartCommandResult OnStartCommand(Android.Content.Intent intent, StartCommandFlags flags, int startId)
        {
            var channelName = "HCChannel";
            var channelDescription = "Home Controller Updates";
            var channel = new NotificationChannel("HCChannelId", channelName, NotificationImportance.Default)
            {
                Description = channelDescription
            };
            //mGoogleApiClient = new GoogleApiClient.Builder(Application.Context, this, this)
            //                     .AddApi(Android.Gms.Location.LocationServices.API)
            //                     .Build();
            //mGoogleApiClient.Connect();


            var notificationManager = (NotificationManager)GetSystemService(NotificationService);
            notificationManager.CreateNotificationChannel(channel);
            var notification = new Notification.Builder(this, "HCChannelId")
            .SetContentTitle("etc")
            .SetContentText("etc")
            .SetOngoing(true)
            .Build();

            StartForeground(SERVICE_RUNNING_NOTIFICATION_ID, notification);
            mRequestedInterval = 0;
            mLog = new HCLogger("192.168.10.10", 8002, "Service.log");
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
            mCommModel.unRegisterCommReceiver(this);
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
            mLog.SendToHost("HomeControlService", "Initialising Location Manager");
            mFusedLocationProviderClient = LocationServices.GetFusedLocationProviderClient(this);
            mLocationCallBack = new LocationCallBack(mLog, this);
            mCurrentUpdateFreqSeconds = HIGH_UPDATE_FREQ_SECONDS;
            mNotMovingStartTime = null;
            Android.Gms.Location.LocationRequest locationRequest = new LocationRequest()
                                  .SetPriority(LocationRequest.PriorityBalancedPowerAccuracy)
                                  .SetInterval(mCurrentUpdateFreqSeconds * 1000) 
                                  .SetFastestInterval(10 * 1000); //Fasted inteval
            mFusedLocationProviderClient.RequestLocationUpdates(locationRequest, mLocationCallBack, null);
            mLog.SendToHost("HomeControlService", "Location Manager Initialised");

            //Listen for updates on the location interval
            mCommModel.registerCommReceiver(this);
        }

        #region location
        public void NewLocation(Location location, double distanceTraveled)
        {
            if (location != null)
            {
                mLog.SendToHost("HomeControlService", string.Format("lat: {0:f6}, lon: {1:f6}, acc:{2}, time:{3}, provider:{4}, distance: {5}", location.Latitude, location.Longitude, location.Accuracy, location.Time, location.Provider, distanceTraveled));
                if (mRequestedInterval == 0)
                {
                    if ((distanceTraveled <= NOT_MOVING_METERS) && mCurrentUpdateFreqSeconds == HIGH_UPDATE_FREQ_SECONDS)
                    {
                        if (mNotMovingStartTime == null)
                        {
                            mNotMovingStartTime = DateTime.Now;
                        }
                        else
                        {
                            if ((DateTime.Now - mNotMovingStartTime.Value).TotalSeconds >= NOT_MOVING_SECONDS)
                            {
                                mLog.SendToHost("HomeControlService", "Switch to low update frequency");
                                mCurrentUpdateFreqSeconds = LOW_UPDATE_FREQ_SECONDS;
                                Android.Gms.Location.LocationRequest locationRequest = new LocationRequest()
                                  .SetPriority(LocationRequest.PriorityBalancedPowerAccuracy)
                                  .SetInterval(mCurrentUpdateFreqSeconds * 1000)
                                  .SetFastestInterval(10 * 1000); //Fasted inteval
                                mFusedLocationProviderClient.RequestLocationUpdates(locationRequest, mLocationCallBack, null);
                                CreateGeofence(location, 100);
                            }

                        }
                    }
                    else
                    {
                        mNotMovingStartTime = null;
                    }
                    if ((distanceTraveled > NOT_MOVING_METERS) && mCurrentUpdateFreqSeconds == LOW_UPDATE_FREQ_SECONDS)
                    {
                        mLog.SendToHost("HomeControlService", "Switch to high update frequency");
                        mCurrentUpdateFreqSeconds = HIGH_UPDATE_FREQ_SECONDS;
                        Android.Gms.Location.LocationRequest locationRequest = new LocationRequest()
                          .SetPriority(LocationRequest.PriorityBalancedPowerAccuracy)
                          .SetInterval(mCurrentUpdateFreqSeconds * 1000)
                          .SetFastestInterval(10 * 1000); //Fasted inteval
                        mFusedLocationProviderClient.RequestLocationUpdates(locationRequest, mLocationCallBack, null);
                        DeleteGeofence();
                    }
                }
                GpsLocation loc = new GpsLocation { Latitude = location.Latitude, Longitude = location.Longitude, Accuracy = location.Accuracy };
                mCommModel.sendObjectQueued(loc);
            }
        }
        #endregion

        #region geofence

        private PendingIntent getGeofencePendingIntent()
        {
            if (mPendingIntent != null)
            {
                return mPendingIntent;
            }
            PendingIntent.GetBroadcast(this, 0, new Intent(this, typeof(HomeControlService)), PendingIntentFlags.UpdateCurrent);
            return mPendingIntent;
        }

        private void CreateGeofence(Location location, int radiusMeters)
        {
            //mLog.SendToHost("HomeControlService", "Create geofence");
            //mGeofenceList.Clear();
            //mGeofenceList.Add(new GeofenceBuilder()
            //           .SetRequestId(GEOFENCEID)
            //           .SetCircularRegion(
            //               location.Latitude,
            //               location.Longitude,
            //               radiusMeters
            //           )
            //           .SetExpirationDuration(60*60*1000)
            //           .SetTransitionTypes(Geofence.GeofenceTransitionEnter | Geofence.GeofenceTransitionExit)
            //           .Build());
            //GeofencingRequest.Builder builder = new GeofencingRequest.Builder();
            //builder.SetInitialTrigger(GeofencingRequest.InitialTriggerEnter);
            //builder.AddGeofences(mGeofenceList);

            //GeofencingRequest request = builder.Build();
            //LocationServices.GeofencingApi.AddGeofences(mGoogleApiClient, request, getGeofencePendingIntent()).SetResultCallback(this);
                        
            ////GeofencingRequest.Builder builder = new GeofencingRequest.Builder();
            ////builder.SetInitialTrigger(GeofencingRequest.InitialTriggerEnter);
            ////builder.AddGeofences(mGeofenceList);

            ////GeofencingRequest request = builder.Build();
            ////if (mPendingIntent == null)
            ////{
            ////    var normalIntent = new Intent(this, typeof(HomeControlService));
            ////    mPendingIntent = PendingIntent.GetService(this, 0, normalIntent, PendingIntentFlags.UpdateCurrent);
            ////}

            ////mGeofencingClient.AddGeofences(request, mPendingIntent);
        }
        private void DeleteGeofence()
        {
            //mLog.SendToHost("HomeControlService", "Delete geofence");

            //if (mPendingIntent != null)
            //{
            //    LocationServices.GeofencingApi.RemoveGeofences(mGoogleApiClient, mPendingIntent);
            //    //mGeofencingClient.RemoveGeofences(mPendingIntent);
            //}
        }
//        protected override void OnHandleIntent(Intent intent)
//        {
//            var geofencingEvent = GeofencingEvent.FromIntent(intent);
//            //mLog.SendToHost("HomeControlService", "Geofence event");
//            Log.Debug("HomeControlService", "Geofence event");
//            if (geofencingEvent.HasError)
//            {
//                var errorMessage = GeofenceErrorMessages.GetErrorString(this, geofencingEvent.ErrorCode);
//                mLog.SendToHost("HomeControlService", errorMessage);
//                return;
//            }

//            int geofenceTransition = geofencingEvent.GeofenceTransition;

//            if (geofenceTransition == Geofence.GeofenceTransitionExit)
//            {
//                mLog.SendToHost("HomeControlService", "Exit geofence: Switch to high update frequency");
//                mCurrentUpdateFreqSeconds = HIGH_UPDATE_FREQ_SECONDS;
//                Android.Gms.Location.LocationRequest locationRequest = new LocationRequest()
//                  .SetPriority(LocationRequest.PriorityHighAccuracy)
//                  .SetInterval(mCurrentUpdateFreqSeconds * 1000)
//                  .SetFastestInterval(10 * 1000); //Fasted inteval
//                mFusedLocationProviderClient.RequestLocationUpdates(locationRequest, mLocationCallBack, null);

//                //IList<IGeofence> triggeringGeofences = geofencingEvent.TriggeringGeofences;

//                //string geofenceTransitionDetails = GetGeofenceTransitionDetails(this, geofenceTransition, triggeringGeofences);

//               // SendNotification(geofenceTransitionDetails);
//             //   Log.Info(TAG, geofenceTransitionDetails);
//            }
//            else
//            {
//                // Log the error.
////                Log.Error(TAG, GetString(Resource.String.geofence_transition_invalid_type, new[] { new Java.Lang.Integer(geofenceTransition) }));
//            }
//        }

        string GetGeofenceTransitionDetails(Context context, int geofenceTransition, IList<IGeofence> triggeringGeofences)
        {
            string geofenceTransitionString = GetTransitionString(geofenceTransition);

            var triggeringGeofencesIdsList = new List<string>();
            foreach (IGeofence geofence in triggeringGeofences)
            {
                triggeringGeofencesIdsList.Add(geofence.RequestId);
            }
            var triggeringGeofencesIdsString = string.Join(", ", triggeringGeofencesIdsList);

            return geofenceTransitionString + ": " + triggeringGeofencesIdsString;
        }

        string GetTransitionString(int transitionType)
        {
            switch (transitionType)
            {
                case Geofence.GeofenceTransitionEnter:
                    return "Entered";
                case Geofence.GeofenceTransitionExit:
                    return "Exited";
                default:
                    return "Unknown transition";
            }
        }
        #endregion

        public void OnStatusChanged(string provider, Availability status, Bundle extras)
        {
            MessageObject msg = new MessageObject();
            msg.Message = string.Format("Provider status changed, provider: {0}, status: {1}", provider, status.ToString());
            mCommModel.sendObjectQueued(msg);
        }

        public void OnConnected(Bundle connectionHint)
        {
            mLog.SendToHost("HomeControlService", "OnConnected");
        }

        public void OnConnectionSuspended(int cause)
        {
            mLog.SendToHost("HomeControlService", "OnConnectionSuspended");
        }

        public void OnConnectionFailed(ConnectionResult result)
        {
            mLog.SendToHost("HomeControlService", "OnConnectionFailed");
        }

        public void OnResult(Java.Lang.Object result)
        {
            mLog.SendToHost("HomeControlService", "OnResult");
        }


        #endregion hcservice
        #region ICommReceiver
        public void receiveObject(object obj)
        {
            LocationInterval interval = obj as Comm.LocationInterval;
            if (interval != null)
            {
                if (mRequestedInterval != interval.interval)
                {
                    mRequestedInterval = interval.interval;
                    int accuracy = LocationRequest.PriorityHighAccuracy;
                    if (mRequestedInterval > 0)
                    {
                        mCurrentUpdateFreqSeconds = mRequestedInterval;
                        accuracy = LocationRequest.PriorityHighAccuracy;
                        mLog.SendToHost("HomeControlService", string.Format("Switch to requested update frequency: {0}", mCurrentUpdateFreqSeconds));
                    }
                    else
                    {
                        mCurrentUpdateFreqSeconds = HIGH_UPDATE_FREQ_SECONDS;
                        accuracy = LocationRequest.PriorityBalancedPowerAccuracy;
                        mLog.SendToHost("HomeControlService", string.Format("Switch default update frequency: {0}", mCurrentUpdateFreqSeconds));
                    }

                    Android.Gms.Location.LocationRequest locationRequest = new LocationRequest()
                      .SetPriority(accuracy)
                      .SetInterval(mCurrentUpdateFreqSeconds * 1000)
                      .SetFastestInterval(10 * 1000); //Fasted inteval
                    mFusedLocationProviderClient.RequestLocationUpdates(locationRequest, mLocationCallBack, null);
                }
                    
            }
        }

        public void connected()
        {
            
        }

        public void disconnected()
        {
            
        }
        #endregion
    }

}