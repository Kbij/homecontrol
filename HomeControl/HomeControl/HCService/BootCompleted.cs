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
    [BroadcastReceiver]
    [IntentFilter(new[] { Intent.ActionBootCompleted })]
    [IntentFilter(new[] { "RestartHCService" })]
    class BootCompleted : BroadcastReceiver
    {
        public override void OnReceive(Context context, Intent intent)
        {
            if (intent.Action == Intent.ActionBootCompleted || intent.Action == "RestartHCService")
            {
                context.ApplicationContext.StartService(new Intent(context, typeof(HomeControlService)));
            }
        }
    }
}