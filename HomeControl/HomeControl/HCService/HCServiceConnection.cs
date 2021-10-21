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
using HomeControl.Comm;

namespace HomeControl.HCService
{
    class HCServiceConnection : Java.Lang.Object, IServiceConnection
    {
        private MainActivity mActivity;
        private HCLogger mLog;
        public HCServiceConnection(MainActivity activity)
        {
            this.mActivity = activity;
            mLog = new HCLogger("192.168.10.11", 8001, "ServiceConnection.log");
            mLog.SendToHost("HCServiceConnection", "HCServiceConnection constructor");
        }

        public void OnServiceConnected(ComponentName name, IBinder service)
        {
            mLog.SendToHost("HCServiceConnection", "OnServiceConnected");
            var serviceBinder = service as HCServiceBinder;
            if (serviceBinder != null)
            {
                mActivity.setBinder(serviceBinder);
                mActivity.onBind();
            }
        }

        public void OnServiceDisconnected(ComponentName name)
        {
            mLog.SendToHost("HCServiceConnection", "OnServiceDisconnected");
            mActivity.unsetBinder();
        }
    }
}