using Android.Util;
using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace HomeControl.Comm
{
    public class RemoteLogClient
    {
        private UdpClient client;

        public RemoteLogClient(string ipAddress, int port)
        {
            //try
            //{
            //    client = new UdpClient();
            //    client.Connect(new IPEndPoint(IPAddress.Parse(ipAddress), port));
            //}
            //catch (Exception) { }
        }

        public void SendToHost(string category, string message)
        {
            //Log to Studio
            Log.Debug(category, message);

            ////Log to network
            //try
            //{
            //    byte[] data = Encoding.ASCII.GetBytes(string.Format("{0}:{1}: {2}/{3}", DateTime.Now, Android.OS.Build.Model, category, message));
            //    client.Send(data, data.Length);
            //}
            //catch (Exception) { }
        }
    }
}