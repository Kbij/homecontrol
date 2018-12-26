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
    public class LocationInterval : ICommObject
    {
        public const byte ObjectID = 30;
        public int interval { get; set; }
        public LocationInterval()
        {
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<LocationInterval>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<LocationInterval>.DeSerialize(json);
            interval = value.interval;
        }
    }
}