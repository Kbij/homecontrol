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
        const string HEADER = "HCM";
        const string SERVER_HOST = "paradijs.mooo.com";
        const int SERVER_PORT = 5678;
        HCLogger mLog = null;
        Socket mSocket;
        Object mLock;
        Guid mId;
        public event EventHandler OnSocketConnected;
        public event EventHandler<FrameReceivedArgs> OnFrameReceived;

        public CloudSocket(HCLogger logger)
        {
            mId = Guid.NewGuid();
            mLog = logger;
            if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("CloudSocket created, this: {0}", mId.ToString().ToUpper()));
            mSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            mBuffer = new List<byte>();
            mSocketState = SocketState.Connecting;
            mLock = new object();
            startThread();
        }
        
        public void Dispose()
        {
            if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("CloudSocket disposed, this: {0}", mId.ToString().ToUpper()));
            stopThread();
        }

        public string Id()
        {
            return mId.ToString().ToUpper();
        }

        private void SocketConnectedEvent()
        {
            EventHandler handler = OnSocketConnected;
            EventArgs e = new EventArgs();
            if (handler != null)
            {
                handler(this, e);
            }
        }

        private void FrameReceivedEvent(int objectId, List<byte> frame)
        {
            EventHandler<FrameReceivedArgs> handler = OnFrameReceived;
            FrameReceivedArgs e = new FrameReceivedArgs();
            e.ObjectId = objectId;
            e.Frame = frame;
            if (handler != null)
            {
                handler(this, e);
            }
        }

        public bool isActive()
        {
           // lock (mLock)
            {
                return (mSocketState == SocketState.Connecting) || (mSocketState == SocketState.Connected);
            }
        }

        public bool sendFrame(byte objectId, List<byte> frame)
        {
            if (mSocket.Connected)
            {
                try
                {
                    List<Byte> sendFrame = new List<byte>();
                    sendFrame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                    if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Sendframe, objectId: {0}, length: {1}, this: {2}", objectId, frame.Count, mId.ToString().ToUpper()));
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
                    if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Exception in sendFrame: {0}, this: {1}", ex.Message, mId.ToString().ToUpper()));
                    lock (mLock)
                    {
                        mSocketState = SocketState.Disconnected;
                        mSocket.Close();
                    }
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
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Exception in stopthread: {0}, this: {1}", ex.Message, mId.ToString().ToUpper()));
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
                        FrameReceivedEvent(objectId, frame);
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
                lock (mLock)
                {
                    mSocketState = SocketState.Connected;
                }

                if (mLog != null) mLog.SendToHost("CloudSocket", "Connected with server");
                SocketConnectedEvent();

                int bytesReceived = 0;
                Byte[] bytes = new Byte[256];
                while ((bytesReceived = mSocket.Receive(bytes)) != 0)
                {
                    if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Bytes received: {0}", bytesReceived));
                    mBuffer.AddRange(bytes.Take(bytesReceived).ToArray());
                    processReceiveBuffer();
                }
            }
            catch (Exception ex)
            {
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Exception in comm thread: {0}, this: {1}", ex.Message, mId.ToString().ToUpper()));
            }
            mSocketState = SocketState.Disconnected;

            if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Done, this: {0}", mId.ToString().ToUpper()));
 
            //Cleaning up
            try
            {
                mSocket.Close();
            }
            catch (Exception ex)
            {
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("Exception in cleanup CloudSocket: {0}, this: {1}", ex.Message, mId.ToString().ToUpper()));
            }
        }
    }
}