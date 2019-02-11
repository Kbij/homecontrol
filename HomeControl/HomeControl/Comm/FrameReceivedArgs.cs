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
    public class FrameReceivedArgs
    {
        public int ObjectId { get; set; }
        public List<byte> Frame { get; set; }
    }
}