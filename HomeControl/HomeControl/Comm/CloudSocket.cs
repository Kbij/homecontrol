using Android.Util;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Threading;

namespace HomeControl.Comm
{
    class CloudSocket : IDisposable
    {
        enum SocketState { Connecting, Connected, Disconnected };
        SocketState mSocketState;
        List<Byte> mBuffer;
        Thread mThread;
        ICloudSocketListener mListener;
        const string HEADER = "HCM";
        const string SERVER_HOST = "paradijs.mooo.com";
        const int SERVER_PORT = 5678;
//        const int SERVER_PORT = 5679;
        //const string SERVER_HOST = "192.168.10.142";
        HCLogger mLog;
        Socket mSocket;

        public CloudSocket(ICloudSocketListener listener, HCLogger logger)
        {
            mLog = logger;
            mLog.SendToHost("CloudSocket", "CloudSocket created");
            mSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            mBuffer = new List<byte>();
            mListener = listener;
            mSocketState = SocketState.Connecting;
            startThread();
        }

        public void Dispose()
        {
            mLog.SendToHost("CloudSocket", "CloudSocket disposed");
            mListener = null;
            stopThread();
        }

        public bool isActive()
        {
            return (mSocketState == SocketState.Connecting) || (mSocketState == SocketState.Connected);
        }

        public bool sendFrame(byte objectId, List<byte> frame)
        {
            if (mSocket.Connected)
            {
                try
                {
                    List<Byte> sendFrame = new List<byte>();
                    sendFrame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                    mLog.SendToHost("CloudSocket", string.Format("Sendframe, objectId: {0}, length: {1}", objectId, frame.Count));
                    int length = frame.Count;
                    int msb = (int)(length / 256);
                    int lsb = length - (msb * 256);
                    sendFrame.Add((byte)msb);
                    sendFrame.Add((byte)lsb);
                    sendFrame.Add(objectId);
                    sendFrame.AddRange(frame);
                    int bytesSend = mSocket.Send(sendFrame.ToArray());

                    //All bytes send ?
                    return (bytesSend == sendFrame.Count);
                }
                catch (Exception ex)
                {
                    mLog.SendToHost("CloudSocket", string.Format("Exception in sendFrame: {0}", ex.Message));
                    mSocketState = SocketState.Disconnected;
                    mSocket.Close();
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
            try
            {
                mSocket.Close();
            }
            catch (Exception ex)
            {
                mLog.SendToHost("CloudSocket", string.Format("Exception in stopthread: {0}", ex.Message));
            }
            mThread.Join();
        }

        private void processReceiveBuffer()
        {
            bool frameFound = true;
            while (frameFound)
            {
                frameFound = false;
                if (mBuffer.Count >= (HEADER.Length + 3))
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
                mSocket.Connect(SERVER_HOST, SERVER_PORT);
                mSocketState = SocketState.Connected;

                mLog.SendToHost("CloudSocket", "Connected with server");
                mListener.socketConnected();

                int bytesReceived = 0;
                Byte[] bytes = new Byte[256];
                while ((bytesReceived = mSocket.Receive(bytes)) != 0)
                {
                    mLog.SendToHost("CloudSocket", string.Format("Bytes received: {0}", bytesReceived));
                    mBuffer.AddRange(bytes.Take(bytesReceived).ToArray());
                    processReceiveBuffer();
                }
            }
            catch (Exception ex)
            {
                mLog.SendToHost("CloudSocket", string.Format("Exception in comm thread: {0}", ex.Message));
            }
            mSocketState = SocketState.Disconnected;

            mLog.SendToHost("CloudSocket", "Done");

            //Cleaning up
            try
            {
                mSocket.Close();
            }
            catch (Exception ex)
            {
                mLog.SendToHost("CloudSocket", string.Format("Exception in cleanup CloudSocket: {0}", ex.Message));
            }
        }
    }
}