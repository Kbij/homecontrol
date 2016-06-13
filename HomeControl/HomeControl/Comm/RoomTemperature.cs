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
    public class RoomTemperature : ICommObject
    {
        public const byte ObjectID = 22;

        public string roomId { get; set; }
        public double roomTemperature { get; set; }
        public RoomTemperature()
        {
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<RoomTemperature>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<RoomTemperature>.DeSerialize(json);
            roomId = value.roomId;
            roomTemperature = value.roomTemperature;
        }
    }
}