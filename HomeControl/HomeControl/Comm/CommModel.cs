using Android.Util;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Threading;

namespace HomeControl.Comm
{
    public class CommModel: IDisposable, ICloudSocketListener
    {
        ICommReceiver mReceiver = null;
        enum CommState { Disconnected, Connecting, NameSend, Connected};
        CommState mCommState;
        int mConnectTimeoutSeconds;
        Queue<ICommObject> mSendQueue;
        RemoteLogClient mLog;
        bool mCommStarted;
        int mOutstandingKeepAlives;
        const int OBJ_KEEPALIVE = 0;
        const int OBJ_HCNAME = 1;
        const int OBJ_SERVERNAME = 2;

        const int KEEPALIVE_INTERVAL_SECONDS = 30;
        const int RECONNECT_SECONDS = 10;
        const int MAX_LOST_KEEPALIVE = 4;
        CloudSocket mCloudSocket;
        Thread mThread;
        bool mThreadRunning = false;
        const int CONNECT_TIMEOUT_SECONDS = 5;

        public CommModel()
        {
            mSendQueue = new Queue<ICommObject>();
            mLog = new RemoteLogClient("192.168.10.10", 8001);
            mLog.SendToHost("CommModel", "CommModel created");
            mCommStarted = false;
        }
        #region public
        public void startComm()
        {
            if (!mCommStarted)
            {
                mCommStarted = true;
                startMaintenanceThread();
                reconnect();
                mLog.SendToHost("CommModel", "CommModel started");
            }
        }
        public void Dispose()
        {
            mLog.SendToHost("CommModel", "Dispose");
            stopMaintenanceThread();
            if (mCloudSocket != null)
            {
                mCloudSocket.Dispose();
            }
        }

        public void sendObjectQueued(ICommObject obj)
        {
            lock(mSendQueue)
            {
                mSendQueue.Enqueue(obj);
            }

            processSendQueue();
        }
        #endregion

        #region socketListener
        public void receiveFrame(int objectId, List<byte> frame)
        {
            switch (objectId)
            {
                case OBJ_KEEPALIVE:
                    mOutstandingKeepAlives = 0;
                    break;
                case OBJ_SERVERNAME:
                    string serverName = System.Text.Encoding.ASCII.GetString(frame.ToArray());
                    if (mCommState == CommState.NameSend)
                    {
                        if (mReceiver != null)
                        {
                            mReceiver.connected();
                        }
                    }
                    mCommState = CommState.Connected;
                    mLog.SendToHost("CommModel", string.Format("Connected with server: {0}", serverName));

                    processSendQueue();
                    break;
                default:
                    string payload = System.Text.Encoding.ASCII.GetString(frame.ToArray());
                    mLog.SendToHost("CommModel", "Payload received");
                    break;
            }
        }

        public void socketDisconnected()
        {
            mLog.SendToHost("CloudComm", "Cloud disconnected, starting reconnect timer");
            if (mReceiver != null)
            {
                mReceiver.disconnected();
            }

            reconnect();
        }

        public void socketConnected()
        {
            string modelName = Android.OS.Build.Model;
            mCloudSocket.sendFrame(OBJ_HCNAME, System.Text.Encoding.ASCII.GetBytes(modelName).ToList());
            mCommState = CommState.NameSend;
        }
        #endregion


        private void reconnect()
        {
            mCommState = CommState.Disconnected;
            mConnectTimeoutSeconds = CONNECT_TIMEOUT_SECONDS;
        }

        private void startConnect()
        {
            mCommState = CommState.Connecting;
            if (mCloudSocket != null)
            {
                mCloudSocket.Dispose();
            }
            mCloudSocket = new CloudSocket(this);
        }

        private void processSendQueue()
        {
            if (mCommState == CommState.Connected)
            {
                bool success = true;
                lock (mSendQueue)
                {
                    while (success && mSendQueue.Count > 0)
                    {
                        success = sendObject(mSendQueue.Peek());
                        if (success)
                        {
                            mSendQueue.Dequeue();
                        }
                    }
                }
            }
        }

        private bool sendObject(ICommObject obj)
        {
            string json = obj.serialise();
            return mCloudSocket.sendFrame(obj.objectId(), System.Text.Encoding.ASCII.GetBytes(json).ToList());
        }

        private void sendKeepAlive(object sender, System.Timers.ElapsedEventArgs e)
        {
            if (mCommState == CommState.Connected)
            {
                mCloudSocket.sendFrame(OBJ_KEEPALIVE, new List<byte>());
            }
        }

        public void registerCommReceiver(ICommReceiver receiver)
        {
            mLog.SendToHost("CommModel", "registerCommReceiver");
            mReceiver = receiver;
            // Sending the current state of the connection
            if (mCommState == CommState.Connected)
            {
                mReceiver.connected();
            }
            else
            {
                mReceiver.disconnected();
            }
        }

        public void unRegisterCommReceiver()
        {
            mLog.SendToHost("CommModel", "unRegisterCommReceiver");
            mReceiver = null;
        }

        private void startMaintenanceThread()
        {
            stopMaintenanceThread();
            mThreadRunning = true;
            mThread = new Thread(maintenanceThread);
            mThread.Name = "SocketThread";
            mThread.Start();
        }

        private void stopMaintenanceThread()
        {
            if (mThreadRunning)
            {
                mThread.Join();
                mThreadRunning = false;
            }
        }

        private void maintenanceThread()
        {
            while (mThreadRunning)
            {
                Thread.Sleep(1000);
                if (mCommState == CommState.Disconnected)
                {
                    --mConnectTimeoutSeconds;
                    if (mConnectTimeoutSeconds <=0)
                    {
                        startConnect();
                    }
                }
            }
        }
    }
}