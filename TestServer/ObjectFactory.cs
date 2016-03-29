using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    public class ObjectFactory
    {
        public static object createObject(byte objectId, string json)
        {
            Object result = null;
            using (var stream = new MemoryStream(Encoding.Default.GetBytes(json)))
            {
                switch (objectId)
                {
                    case GpsLocation.ObjectID:
                        {
                            DataContractJsonSerializer serializer = new DataContractJsonSerializer(typeof(GpsLocation));
                            result = (GpsLocation)serializer.ReadObject(stream);
                            break;
                        }

                    case MessageObject.ObjectID:
                        {
                            DataContractJsonSerializer serializer = new DataContractJsonSerializer(typeof(MessageObject));
                            result = (MessageObject)serializer.ReadObject(stream);
                            break;
                        }
                }
            }

            return result;
        }
    }
}
