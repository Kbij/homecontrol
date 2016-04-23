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
using System.Threading;
using System.Net.Sockets;
using Android.Util;

namespace HomeControl.Comm
{
    class CloudSocket : IDisposable
    {
        List<Byte> mBuffer;
        Thread mThread;
        ICloudSocketListener mListener;
        const string HEADER = "HCM";
        const string SERVER_HOST = "paradijs.mooo.com";
        NetworkStream mStream;
        TcpClient mClient;
        RemoteLogClient mLog;

        public CloudSocket(ICloudSocketListener listener)
        {
            mLog = new RemoteLogClient("192.168.10.10", 8001);
            mLog.SendToHost("CloudSocket", "CloudSocket created");

            mClient = new TcpClient();
            mBuffer = new List<byte>();
            mListener = listener;
            startThread();
        }

        public void Dispose()
        {
            mListener = null;
            stopThread();
        }

        public bool sendFrame(byte objectId, List<byte> frame)
        {
            if (mStream != null)
            {
                try
                {
                    List<Byte> sendFrame = new List<byte>();
                    sendFrame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));

                    int length = frame.Count;
                    int msb = (int)(length / 256);
                    int lsb = length - (msb * 256);
                    sendFrame.Add((byte)msb);
                    sendFrame.Add((byte)lsb);
                    sendFrame.Add(objectId);
                    sendFrame.AddRange(frame);
                    mStream.Write(sendFrame.ToArray(), 0, sendFrame.Count);
                    return true;
                }
                catch (Exception ex)
                {
                    Log.Debug("CloudSocket", string.Format("Exception in sendFrame: {0}", ex.Message));
                }

            }
            return false;
        }

        private void startThread()
        {
            mThread = new Thread(commThread);
            mThread.Name = "SocketThread";
            mThread.Start();
        }

        private void stopThread()
        {
            mThread.Join();
        }

        private void processReceiveBuffer()
        {
            bool frameFound = true;
            while (frameFound)
            {
                frameFound = false;
                if (mBuffer.Count > (HEADER.Length + 3))
                {
                    int length = mBuffer[HEADER.Length] * 256;
                    length += mBuffer[HEADER.Length + 1];
                    int objectId = mBuffer[HEADER.Length + 2];
                    if (mBuffer.Count >= (length + HEADER.Length + 3))
                    {
                        frameFound = true;
                        List<byte> frame = new List<byte>(mBuffer.GetRange(HEADER.Length + 3, length));
                        mListener.receiveFrame(objectId, frame);

                        mBuffer.RemoveRange(0, length + HEADER.Length + 3);
                    }
                }
            }
        }

        private void commThread()
        {
            // Using a synchronous model
            try
            {
                using (TcpClient client = new TcpClient())
                {
                    client.Connect(SERVER_HOST, 5678);
                    mLog.SendToHost("CloudSocket", "Connected with server");
                    mStream = client.GetStream();
                    mListener.socketConnected();

                    int bytesReceived = 0;
                    Byte[] bytes = new Byte[256];
                    while ((bytesReceived = mStream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        mBuffer.AddRange(bytes.Take(bytesReceived).ToArray());
                        processReceiveBuffer();
                    }
                }

            }
            catch (Exception ex)
            {
                mLog.SendToHost("CloudSocket", string.Format("Exception in comm thread: {0}", ex.Message));
            }
            mLog.SendToHost("CloudSocket", "Done");

            //Cleaning up
            try
            {
                if (mListener != null)
                {
                    mListener.socketDisconnected();
                }

                if (mStream != null)
                {
                    mStream.Close();
                }

                mClient.Close();
            }
            catch (Exception ex)
            {
                mLog.SendToHost("CloudSocket", string.Format("Exception in cleanup CloudSocket: {0}", ex.Message));
            }
        }
    }
}