using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    public class MessageObject
    {
        public const byte ObjectID = 11;

        public string Message { get; set; }
        public DateTime TimeStamp { get; set; }
        public string toString()
        {
            return string.Format("Message: {0}, Time: {1}", Message, TimeStamp);
        }
    }
}
