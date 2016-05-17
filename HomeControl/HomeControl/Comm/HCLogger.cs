using Android.Util;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace HomeControl.Comm
{
    public class HCLogger 
    {
        private UdpClient client;
        private string mFileName;
        public HCLogger(string ipAddress, int port)
        {
            //string path = Environment.GetFolderPath(Environment.SpecialFolder.);
            mFileName = "/sdcard/Android/data/HomeControl.HomeControl/files/HomeControl.log";//Path.Combine(path, "HomeControl.log");

            try
            {
                //client = new UdpClient();
                //client.Connect(new IPEndPoint(IPAddress.Parse(ipAddress), port));

            }
            catch (Exception) { }

            SendToHost("HCLog", "Path:" + mFileName);
        }

        public void SendToHost(string category, string message)
        {
            //Log to Studio
            Log.Debug(category, "HCLog:" + message);

            // log to file
            try
            {
               using (var streamWriter = new StreamWriter(mFileName, true))
                {
                    streamWriter.WriteLine(string.Format("{0}:{1}: {2}/{3}", DateTime.Now, Android.OS.Build.Model, category, message));
                }
            }
            catch (Exception) { }

            //Log to network
            try
            {
                //byte[] data = Encoding.ASCII.GetBytes(string.Format("{0}:{1}: {2}/{3}", DateTime.Now, Android.OS.Build.Model, category, message));
                //client.Send(data, data.Length);
            }
            catch (Exception) { }
        }
    }
}