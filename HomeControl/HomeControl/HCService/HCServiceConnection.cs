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

namespace HomeControl.HCService
{
    class HCServiceConnection : Java.Lang.Object, IServiceConnection
    {
        MainActivity activity;

        public HCServiceConnection(MainActivity activity)
        {
            this.activity = activity;
        }

        public void OnServiceConnected(ComponentName name, IBinder service)
        {
            var hcServiceBinder = service as HCServiceBinder;
            if (hcServiceBinder != null)
            {
                activity.binder = hcServiceBinder;
                activity.isBound = true;
                activity.onBind();
            }
        }

        public void OnServiceDisconnected(ComponentName name)
        {
            activity.isBound = false;
            //activity.onUnBind();
        }
    }
}