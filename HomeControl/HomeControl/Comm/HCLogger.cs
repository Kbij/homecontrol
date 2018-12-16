using Android.Util;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.CompilerServices;
using System.Text;

namespace HomeControl.Comm
{
    public class HCLogger 
    {
        private UdpClient client;
        private string mFileName;
        private object mLock;
        public HCLogger(string ipAddress, int port, string fileName)
        {
            //string path = Environment.GetFolderPath(Environment.SpecialFolder.);
            //mFileName = string.Format("/sdcard/Android/data/HomeControl.HomeControl/files/{0}", fileName);//Path.Combine(path, "HomeControl.log");
            string path = Android.OS.Environment.ExternalStorageDirectory.AbsolutePath;
            mFileName = Path.Combine(path, fileName);

            try
            {
                client = new UdpClient();
                client.Connect(new IPEndPoint(IPAddress.Parse(ipAddress), port));

            }
            catch (Exception) { }
            mLock = new object();
            SendToHost("HCLog", "Path:" + mFileName);
        }
        public void Send(string message, [CallerFilePath] string file = "", [CallerMemberName] string member = "", [CallerLineNumber] int line = 0)
        {
            SendToHost(file, message);
        }
        public void SendToHost(string category, string message)
        {
            //Log to Studio
            Log.Debug(category, "HCLog:" + message);

            // log to file
            try
            {
                lock (mLock)
                {
                    using (var streamWriter = new StreamWriter(mFileName, true))
                    {
                        streamWriter.WriteLine(string.Format("{0}:{1}: {2}/{3}", DateTime.Now, Android.OS.Build.Model, category, message));
                    }
                }
            }
            catch (Exception) { }

            //Log to network
            try
            {
                byte[] data = Encoding.ASCII.GetBytes(string.Format("{0}:{1}: {2}/{3}", DateTime.Now, Android.OS.Build.Model, category, message));
                client.Send(data, data.Length);
            }
            catch (Exception ex)
            {
                Log.Debug(category, "HCLog send Exception:" + ex.Message);
            }
        }
    }
}