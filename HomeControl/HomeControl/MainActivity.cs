using System;
using Android.App;
using Android.Content;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.OS;
using HomeControl.HCService;
using HomeControl.Comm;

namespace HomeControl
{
    [Activity(Label = "Home Control", MainLauncher = true, Icon = "@drawable/icon")]
    public class MainActivity : Activity, ICommReceiver
    {
        public HCServiceBinder binder;
        public bool isBound;
        HCServiceConnection hcServiceConnection;

        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.Main);
            StartService(new Intent(this, typeof(HomeControlService)));
        }
        protected override void OnStart()
        {
            base.OnStart();
            Intent hcServiceIntent = new Intent(this, typeof(HomeControlService));

            hcServiceConnection = new HCServiceConnection(this);
            if (BindService(hcServiceIntent, hcServiceConnection, Bind.AutoCreate))
            {
               // Log.Debug(TAG, "Service binded");
            }
            else
            {
                //Log.Debug(TAG, "Service NOT binded");
            }
        }

        protected override void OnStop()
        {
            if (isBound)
            {
                binder.GetHCService().unRegisterCommReceiver();
                UnbindService(hcServiceConnection);
                isBound = false;
            }
            base.OnStop();
        }

        public void onBind()
        {
            if (isBound)
            {
                binder.GetHCService().registerCommReceiver(this);
            }
        }

        public void receiveObject(object obj)
        {

        }

        public void connected()
        {
//            Action action = delegate { this.Title = "Home Control (connected)"; };
            // gridview.Post(action);
            RunOnUiThread(() => this.Title = "Home Control (connected)");
        }

        public void disconnected()
        {
            //          Action action = delegate { this.Title = "Home Control (connected)"; };
            RunOnUiThread(() => this.Title = "Home Control");
        }
    }
}

