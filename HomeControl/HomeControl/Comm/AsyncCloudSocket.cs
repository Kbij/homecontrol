using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;

namespace HomeControl.Comm
{
    class AsyncCloudSocket : IDisposable
    {
        enum SocketState { Connecting, ConnectionFailed, Connected, Disconnected };
        Guid mId;
        HCLogger mLog = null;
        const string HEADER = "HCM";
        const string SERVER_HOST = "paradijs.mooo.com";
        const int SERVER_PORT = 5678;
        protected Socket mSocket;
        private byte[] mReceiveBuffer;
        private byte[] mSendBuffer;
        List<Byte> mBuffer;
        SocketState mSocketState;
        public event EventHandler OnSocketConnected;
        public event EventHandler<FrameReceivedArgs> OnFrameReceived;

        public AsyncCloudSocket(HCLogger logger)
        {
            mId = Guid.NewGuid();
            mLog = logger;
            mReceiveBuffer = new byte[1024];
            mSocketState = SocketState.Connecting;
            mBuffer = new List<byte>();
            IPAddress ipAddress = null;
            IPEndPoint remoteEP = null;
            try
            {
                IPHostEntry ipHostInfo = Dns.GetHostEntry(SERVER_HOST);
                ipAddress = ipHostInfo.AddressList[0];
                remoteEP = new IPEndPoint(ipAddress, SERVER_PORT);
            }
            catch (Exception ex)
            {
                mSocketState = SocketState.ConnectionFailed;
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[0] Dns failed: {1}", Id(), ex.Message));
            }

            if (ipAddress != null && remoteEP != null)
            { 
                // Create a TCP/IP socket.  
                mSocket = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                // Connect to the remote endpoint.  
                mSocket.BeginConnect(remoteEP, new AsyncCallback(ConnectCallback), null);
            }
        }

        public void Dispose()
        {
        }

        public bool sendFrame(byte objectId, List<byte> frame)
        {
            if (mSocketState == SocketState.Connected)
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
                    mSendBuffer = sendFrame.ToArray();

                    if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Sendframe, objectId: {1}, length: {2}, id: {3}, Frame: {4}",
                                                      Id(), objectId, frame.Count, mId.ToString().ToUpper(), Encoding.ASCII.GetString(mSendBuffer)));

                    // Begin sending the data to the remote device.  
                    mSocket.BeginSend(mSendBuffer, 0, sendFrame.Count, 0, new AsyncCallback(SendCallback), null);
                    return true;
                }
                catch (Exception ex)
                {
                    if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Exception in sendFrame: {1}, this: {2}", Id(), ex.Message, mId.ToString().ToUpper()));
                    mSocketState = SocketState.ConnectionFailed;
                }
            }

            return false;
        }

        public string Id()
        {
            return mId.ToString().ToUpper();
        }

        public bool isActive()
        {
            return (mSocketState == SocketState.Connecting) || (mSocketState == SocketState.Connected);
        }

        private void ConnectCallback(IAsyncResult ar)
        {
            try
            {
                mSocket.EndConnect(ar);

                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Connected with server: {1}", Id(), mSocket.RemoteEndPoint.ToString()));
                mSocket.BeginReceive(mReceiveBuffer, 0, mReceiveBuffer.Length, 0, new AsyncCallback(ReceiveCallBack), null);
                mSocketState = SocketState.Connected;

                OnSocketConnected?.Invoke(this, new EventArgs());
            }
            catch (Exception ex)
            {
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Exception in connectcallback: {1}", Id(), ex.Message ));
                mSocketState = SocketState.ConnectionFailed;
            }
        }

        private void ReceiveCallBack(IAsyncResult ar)
        {
            try
            {
                int bytesRead = mSocket.EndReceive(ar);
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Receivecallback, bytes received: {1}", Id(), bytesRead));

                if (bytesRead > 0)
                {
                    mBuffer.AddRange(mReceiveBuffer.Take(bytesRead).ToArray());
                    processReceiveBuffer();

                    // Get the rest of the data.  
                    mSocket.BeginReceive(mReceiveBuffer, 0, mReceiveBuffer.Length, 0, new AsyncCallback(ReceiveCallBack), null);
                }
            }
            catch (Exception ex)
            {
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Exception in receivecallback: {1}", Id(), ex.Message));
                mSocketState = SocketState.Disconnected;
            }
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

                        OnFrameReceived?.Invoke(this, new FrameReceivedArgs { ObjectId = objectId, Frame = frame});

                        mBuffer.RemoveRange(0, length + HEADER.Length + 3);
                    }
                }
            }
        }

        private void SendCallback(IAsyncResult ar)
        {
            try
            {
                int bytesSent =  mSocket.EndSend(ar);
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] SendCallback, bytes send: {1}", Id(), bytesSent));
            }
            catch (Exception ex)
            {
                if (mLog != null) mLog.SendToHost("CloudSocket", string.Format("[{0}] Exception in sendcallback: {1}", Id(), ex.Message));
                mSocketState = SocketState.Disconnected;
            }
        }
    }
}