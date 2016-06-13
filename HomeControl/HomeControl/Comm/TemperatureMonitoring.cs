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
    public class TemperatureMonitoring : ICommObject
    {
        public const byte ObjectID = 20;

        public bool monitoringEnabled { get; set; }
        public TemperatureMonitoring()
        {
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<TemperatureMonitoring>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<TemperatureMonitoring>.DeSerialize(json);
            monitoringEnabled = value.monitoringEnabled;
        }
    }
}