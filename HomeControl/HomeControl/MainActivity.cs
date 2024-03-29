﻿using Android;
using Android.App;
using Android.Content;
using Android.Content.PM;
using Android.Gms.Common;
using Android.OS;
using Android.Support.V4.App;
using Android.Support.V4.Content;
using Android.Views;
using Android.Widget;
using HomeControl.Comm;
using HomeControl.HCService;
using HomeControl.Logic;
using System;
using System.Collections.Generic;
using System.IO;
using RoomTemperature = HomeControl.Logic.RoomTemperature;

namespace HomeControl
{
    [Activity(Label = "Home Control", MainLauncher = true, LaunchMode = LaunchMode.SingleInstance, Icon = "@drawable/icon")]
    public class MainActivity : Activity, ICommReceiver
    {
        private HCServiceBinder mBinder;
        private bool mIsBound;
        private HCServiceConnection mServiceConnection;
        private GridView mGridView;
        private HCLogger mLog;
        private const int REQUEST_PERMISSIONS = 1;
        private bool mWasAlreadyStarted = false;
        static void HandleExceptions(object sender, UnhandledExceptionEventArgs ex)
        {
            string fileName = "/sdcard/Android/data/HomeControl.HomeControl/files/HomeControlUnhandled.log";//Path.Combine(path, "HomeControl.log");

            using (var streamWriter = new StreamWriter(fileName, true))
            {
                streamWriter.WriteLine(string.Format("{0}:{1}", DateTime.Now, ex.ToString()));
            }
        }

        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);
            mLog = new HCLogger("192.168.10.11", 8001, "MainService.log");
            mLog.SendToHost("MainService", "OnCreate");
            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.UnhandledException += HandleExceptions;

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.Main);
            mGridView = FindViewById<GridView>(Resource.Id.gridView);
            mGridView.Adapter = new TemperatureAdapter(this, TemperatureModel.Instance);
            mGridView.ItemClick += listViewItemClick;
        }

        protected override void OnStart()
        {
            mLog.SendToHost("MainService", "OnStart");
            base.OnStart();
            if (!mWasAlreadyStarted)
            {
                if (!IsGooglePlayServicesInstalled())
                {
                    mLog.Send("Google play services are not installed (should not happen)");
                }

                if (CheckPermissions())
                {   //Start immediately if all permissions are ok
                    startHomeControlService();
                }
            }
            mWasAlreadyStarted = true;
        }

        protected override void OnStop()
        {
            mLog.SendToHost("MainService", "OnStop");
            if (mIsBound && mBinder != null)
            {
                //mLog.SendToHost("MainService", "OnStop, unbind");
                //mBinder.GetHCService().unRegisterCommReceiver(TemperatureModel.Instance);
                //mBinder.GetHCService().unRegisterCommReceiver(this);
                //TemperatureModel.Instance.unsetCommService();
                //UnbindService(mServiceConnection);
                //mIsBound = false;
            }
            base.OnStop();
        }

        protected override void OnDestroy()
        {
            mLog.SendToHost("MainService", "OnDestroy");
            if (mIsBound && mBinder != null)
            {
                mLog.SendToHost("MainService", "OnDestroy, unbind");
                mBinder.GetHCService().unRegisterCommReceiver(TemperatureModel.Instance);
                mBinder.GetHCService().unRegisterCommReceiver(this);
                TemperatureModel.Instance.unsetCommService();
                UnbindService(mServiceConnection);
                mIsBound = false;
            }
            base.OnDestroy();
        }

        protected override void OnResume()
        {
            base.OnResume();
            mLog.SendToHost("MainService", "OnResume");
        }
        #region permissions
        bool CheckPermissions()
        {
            List<string> requiredPermissions = new List<string>();

            //Do we have the permissions ?
            if (ContextCompat.CheckSelfPermission(this, Manifest.Permission.AccessFineLocation) != (int)Permission.Granted)
            {
                // Are we allowed to ask (did the user deny forever ?
                if (ActivityCompat.ShouldShowRequestPermissionRationale(this, Manifest.Permission.AccessFineLocation))
                {
                    requiredPermissions.Add(Manifest.Permission.AccessFineLocation);
                }
                else
                {
                    requiredPermissions.Add(Manifest.Permission.AccessFineLocation);
                }
            }
            if (ContextCompat.CheckSelfPermission(this, Manifest.Permission.WriteExternalStorage) != (int)Permission.Granted)
            {
                if (ActivityCompat.ShouldShowRequestPermissionRationale(this, Manifest.Permission.WriteExternalStorage))
                {
                    requiredPermissions.Add(Manifest.Permission.WriteExternalStorage);
                }
                else
                {
                    requiredPermissions.Add(Manifest.Permission.WriteExternalStorage);
                }
            }
            if (requiredPermissions.Count > 0)
            {
                ActivityCompat.RequestPermissions(this, requiredPermissions.ToArray(), REQUEST_PERMISSIONS);
                return false;
            }
            return true;
        }

        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, Permission[] grantResults)
        {
            if (requestCode == REQUEST_PERMISSIONS)
            {
                // Received permission result for camera permission.
                mLog.SendToHost("MainService", "Received response for Location permission request.");

                bool allGranted = true;
                foreach(var grant in grantResults)
                {
                    allGranted = allGranted && (grant == Permission.Granted);
                }
                if (allGranted)
                {
                    mLog.SendToHost("MainService", "All permissions have been grated.");
                    startHomeControlService();
                }
                else
                {
                    mLog.SendToHost("MainService", "Not all permissions where granted");
                }
            }
            else
            {
                base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
            }
        }
        #endregion

        void startHomeControlService()
        {
            StartForegroundService(new Intent(this, typeof(HomeControlService)));

            mLog.SendToHost("MainService", "OnStart");
            Intent hcServiceIntent = new Intent(this, typeof(HomeControlService));

            mServiceConnection = new HCServiceConnection(this);
            if (BindService(hcServiceIntent, mServiceConnection, Bind.AutoCreate))
            {
            }
            mIsBound = true;
        }

        bool IsGooglePlayServicesInstalled()
        {
            var queryResult = GoogleApiAvailability.Instance.IsGooglePlayServicesAvailable(this);
            if (queryResult == ConnectionResult.Success)
            {
                mLog.SendToHost("MainActivity", "Google Play Services is installed on this device.");
                return true;
            }

            if (GoogleApiAvailability.Instance.IsUserResolvableError(queryResult))
            {
                // Check if there is a way the user can resolve the issue
                var errorString = GoogleApiAvailability.Instance.GetErrorString(queryResult);
                mLog.SendToHost("MainActivity", string.Format("There is a problem with Google Play Services on this device: {0}- {1}", queryResult, errorString));

                // Alternately, display the error to the user.
            }

            return false;
        }

        public void onBind()
        {
            if (mIsBound)
            {
                mBinder.GetHCService().registerCommReceiver(this);
                mBinder.GetHCService().registerCommReceiver(TemperatureModel.Instance);
                TemperatureModel.Instance.setCommService(mBinder.GetHCService());
            }
        }

        public void setBinder(HCServiceBinder binder)
        {
            mLog.SendToHost("MainService", "SetBinder");
            mBinder = binder;
            mIsBound = true;
        }

        public void unsetBinder()
        {
            mLog.SendToHost("MainService", "UnBinder");
            mIsBound = false;
        }

        public void receiveObject(object obj)
        {
        }

        public void connected()
        {
            mLog.SendToHost("MainService", "Connected");
            RunOnUiThread(() => this.Title = "Home Control (connected)");
        }

        public void disconnected()
        {
            mLog.SendToHost("MainService", "Disconnected");
            RunOnUiThread(() => this.Title = "Home Control");
        }

        public void listViewItemClick(object sender, AdapterView.ItemClickEventArgs args)
        {
            var intent = new Intent(this, typeof(RoomDetail));
            RoomTemperature room = TemperatureModel.Instance.roomTemperatues()[args.Position];
            if (room != null)
            {
                intent.PutExtra("roomId", room.roomId);
                StartActivity(intent);
            }

        }
    }
}

