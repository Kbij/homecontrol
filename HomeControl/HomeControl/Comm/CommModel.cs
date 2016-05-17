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
        HCLogger mLog;
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
        const int CONNECT_TIMEOUT_SECONDS = 30;
        int mLastObjectSendSeconds;
        private Object mLock;

        public CommModel(HCLogger logger)
        {
            mOutstandingKeepAlives = 0;
            mLock = new Object();
            mSendQueue = new Queue<ICommObject>();
            mLog = logger;
            mLog.SendToHost("CommModel", "CommModel created");
            mCommStarted = false;
        }
        #region public
        public void startComm()
        {
            if (!mCommStarted)
            {
                mCommStarted = true;
                mCommState = CommState.Disconnected;
                mConnectTimeoutSeconds = CONNECT_TIMEOUT_SECONDS;

                startMaintenanceThread();
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
                    mLog.SendToHost("CommModel", "keepalive received");
                    --mOutstandingKeepAlives;
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

        public void socketConnected()
        {
            lock (mLock)
            {
                mLastObjectSendSeconds = 0;
                string modelName = Android.OS.Build.Model;
                mCommState = CommState.NameSend;
                mCloudSocket.sendFrame(OBJ_HCNAME, System.Text.Encoding.ASCII.GetBytes(modelName).ToList());
            }
        }
        #endregion

        private void startConnect()
        {
            mLog.SendToHost("CommModel", "startConnect");
            mOutstandingKeepAlives = 0;

            mCommState = CommState.Connecting;
            if (mCloudSocket != null)
            {
                mCloudSocket.Dispose();
            }
            mCloudSocket = new CloudSocket(this, mLog);
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
                        else
                        {
                            mLog.SendToHost("CommModel", "Object not send, not removed from queue");
                        }
                    }
                }
            }
        }

        private bool sendObject(ICommObject obj)
        {
            mLastObjectSendSeconds = 0;
            string json = obj.serialise();
            return mCloudSocket.sendFrame(obj.objectId(), System.Text.Encoding.ASCII.GetBytes(json).ToList());
        }

        private void sendKeepAlive()
        {
            if (mCommState == CommState.Connected)
            {
                if (mOutstandingKeepAlives > 0)
                {
                    mLog.SendToHost("CommModem", string.Format("Keepalive missing ({0}), reconnecting", mOutstandingKeepAlives));

                    mCommState = CommState.Disconnected;
                    mConnectTimeoutSeconds = CONNECT_TIMEOUT_SECONDS;
                    if (mReceiver != null)
                    {
                        mReceiver.disconnected();
                    }
                }
                else
                {
                    mLog.SendToHost("CommModem", "sending keepalive");
                    mLastObjectSendSeconds = 0;
                    mCloudSocket.sendFrame(OBJ_KEEPALIVE, new List<byte>());
                    ++mOutstandingKeepAlives;
                }
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
                try
                {
                   Thread.Sleep(1000);
                   lock (mLock)
                    {

                        if (mCommState == CommState.Disconnected)
                        {
                            --mConnectTimeoutSeconds;
                            if (mConnectTimeoutSeconds <= 0)
                            {
                                startConnect();
                            }
                        }

                        if (mCloudSocket != null)
                        {
                            if (mCommState == CommState.Connected && mCloudSocket.isActive())
                            {
                                ++mLastObjectSendSeconds;
                                if (mLastObjectSendSeconds > KEEPALIVE_INTERVAL_SECONDS)
                                {
                                    sendKeepAlive();
                                }
                            }

                            // No longer active (connected failed, or disconnected
                            if (!mCloudSocket.isActive())
                            {
                                mLog.SendToHost("CommModel", "Cloudsocket no longer active, restarting connection");
                                if (mReceiver != null)
                                {
                                    mReceiver.disconnected();
                                }
                                mCommState = CommState.Disconnected;
                                mConnectTimeoutSeconds = CONNECT_TIMEOUT_SECONDS;

                                mCloudSocket.Dispose();
                                mCloudSocket = null;
                            }
                        }
                    }
                }

                catch (Exception ex )
                {
                    mLog.SendToHost("CommModel", "Maintenance thread exception:" + ex.Message);
                }
            }
        }
    }
}