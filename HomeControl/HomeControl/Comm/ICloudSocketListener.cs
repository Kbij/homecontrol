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

namespace HomeControl.Comm
{
    public interface ICloudSocketListener
    {
        void socketConnected();
        void socketDisconnected();

        void receiveFrame(int objectId, List<byte> frame);
    }
}