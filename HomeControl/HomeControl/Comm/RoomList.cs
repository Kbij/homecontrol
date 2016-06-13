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
    public class Room
    {
        public string roomId { get; set; }
        public string name { get; set; }
        public double roomTemperature { get; set; }
        public double setTemperature { get; set; }

    }
    public class RoomList : ICommObject
    {
        public const byte ObjectID = 21;

        public List<Room> rooms;
        public RoomList()
        {
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<RoomList>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<RoomList>.DeSerialize(json);
            rooms = value.rooms;
        }
    }
}