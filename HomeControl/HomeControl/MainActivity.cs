using System;
using Android.App;
using Android.Content;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.OS;
using HomeControl.HCService;
using HomeControl.Comm;
using System.IO;
using HomeControl.Logic;

namespace HomeControl
{
    [Activity(Label = "Home Control", MainLauncher = true, Icon = "@drawable/icon")]
    public class MainActivity : Activity, ICommReceiver
    {
        private HCServiceBinder mBinder;
        private bool mIsBound;
        private HCServiceConnection mServiceConnection;
        private GridView mGridView;
        private TemperatureModel mTemperatureModel;
        private HCLogger mLog;
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
            mLog = new HCLogger("192.168.10.10", 8001, "MainService.log");
            mLog.SendToHost("MainService", "OnCreate");
            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.UnhandledException += HandleExceptions;

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.Main);
            mTemperatureModel = new TemperatureModel();
            mGridView = FindViewById<GridView>(Resource.Id.gridView);
            mGridView.Adapter = new TemperatureAdapter(this, mTemperatureModel);

            StartService(new Intent(this, typeof(HomeControlService)));
        }
        protected override void OnStart()
        {
            base.OnStart();
            mLog.SendToHost("MainService", "OnStart");
            Intent hcServiceIntent = new Intent(this, typeof(HomeControlService));

            mServiceConnection = new HCServiceConnection(this);
            if (BindService(hcServiceIntent, mServiceConnection, Bind.AutoCreate))
            {
                
               // Log.Debug(TAG, "Service binded");
            }
            else
            {
                //Log.Debug(TAG, "Service NOT binded");
            }
            mIsBound = true;
        }

        protected override void OnStop()
        {
            mLog.SendToHost("MainService", "OnStop");
            if (mIsBound && mBinder != null)
            {
                mLog.SendToHost("MainService", "OnStop, unbind");
                mBinder.GetHCService().unRegisterCommReceiver(mTemperatureModel);
                mBinder.GetHCService().unRegisterCommReceiver(this);
                mTemperatureModel.unsetCommService();
                UnbindService(mServiceConnection);
                mIsBound = false;
            }
            base.OnStop();
        }

        protected override void OnDestroy()
        {
            mLog.SendToHost("MainService", "OnDestroy");
            if (mIsBound && mBinder != null)
            {
                mLog.SendToHost("MainService", "OnDestroy, unbind");
                mBinder.GetHCService().unRegisterCommReceiver(mTemperatureModel);
                mBinder.GetHCService().unRegisterCommReceiver(this);
                mTemperatureModel.unsetCommService();
                UnbindService(mServiceConnection);
                mIsBound = false;
            }
            base.OnDestroy();
        }

        public void onBind()
        {
            if (mIsBound)
            {
                mBinder.GetHCService().registerCommReceiver(this);
                mBinder.GetHCService().registerCommReceiver(mTemperatureModel);
                mTemperatureModel.setCommService(mBinder.GetHCService());
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
    }
}

