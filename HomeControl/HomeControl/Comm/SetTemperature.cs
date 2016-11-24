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
    public class SetTemperature : ICommObject
    {
        public const byte ObjectID = 22;

        public string roomId { get; set; }
        public double setTemperature { get; set; }
        public SetTemperature()
        {
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<SetTemperature>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<SetTemperature>.DeSerialize(json);
            roomId = value.roomId;
            setTemperature = value.setTemperature;
        }
    }
}