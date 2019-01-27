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
    public class TemperatureDown : ICommObject
    {
        public const byte ObjectID = 31;
        public string roomId { get; set; }
        public TemperatureDown()
        {
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<TemperatureDown>.DeSerialize(json);
            roomId = value.roomId;
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<TemperatureDown>.Serialize(this);
        }
    }
}