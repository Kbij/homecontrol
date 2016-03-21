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
            try
            {
                client = new UdpClient();
                client.Connect(new IPEndPoint(IPAddress.Parse(ipAddress), port));
            }
            catch (Exception) { }
        }

        public void SendToHost(string message)
        {
            try
            {
                byte[] data = Encoding.ASCII.GetBytes(string.Format("{0}: {1}", Android.OS.Build.Model, message));
                client.Send(data, data.Length);
            }
            catch (Exception) { }
        }
    }
}