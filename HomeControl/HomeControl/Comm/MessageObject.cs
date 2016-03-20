using System;

namespace HomeControl.Comm
{
    public class MessageObject : ICommObject
    {
        public const byte ObjectID = 11;

        public string Message { get; set; }
        public DateTime TimeStamp { get; set; }
        public MessageObject()
        {
            TimeStamp = DateTime.Now;
        }

        public byte objectId()
        {
            return ObjectID;
        }

        public string serialise()
        {
            return JsonSerialiser<MessageObject>.Serialize(this);
        }

        public void deserialise(string json)
        {
            var value = JsonSerialiser<MessageObject>.DeSerialize(json);
            Message = value.Message;
            TimeStamp = value.TimeStamp;
        }
    }
}