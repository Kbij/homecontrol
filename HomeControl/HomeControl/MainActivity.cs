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
    [Activity(Label = "HomeControl", MainLauncher = true, Icon = "@drawable/icon")]
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

        }

        public void disconnected()
        {

        }
    }
}

