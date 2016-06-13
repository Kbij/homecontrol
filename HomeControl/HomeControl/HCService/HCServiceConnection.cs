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
        private MainActivity mActivity;

        public HCServiceConnection(MainActivity activity)
        {
            this.mActivity = activity;
        }

        public void OnServiceConnected(ComponentName name, IBinder service)
        {
            var serviceBinder = service as HCServiceBinder;
            if (serviceBinder != null)
            {
                mActivity.setBinder(serviceBinder);
                mActivity.onBind();
            }
        }

        public void OnServiceDisconnected(ComponentName name)
        {
            mActivity.unsetBinder();
        }
    }
}