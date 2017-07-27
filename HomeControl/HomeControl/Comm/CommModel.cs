using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace HomeControl.Comm
{
    public class CommModel: IDisposable
    {
        List<ICommReceiver> mReceivers;
        enum CommState {Init, Disconnected, Connecting, NameSend, Connected};
        CommState mCommState = CommState.Init;
        int mConnectTimeoutSeconds;
        Queue<ICommObject> mSendQueue;
        HCLogger mLog;
        bool mCommStarted;
        int mOutstandingKeepAlives;
        const int OBJ_KEEPALIVE = 0;
        const int OBJ_HCNAME = 1;
        const int OBJ_SERVERNAME = 2;

        const int OBJ_ROOMLIST = 21;
        const int OBJ_ROOM_TEMPERATURE = 22;
        const int OBJ_SET_TEMPERATURE = 23;

        const int KEEPALIVE_INTERVAL_SECONDS = 30;
        const int RECONNECT_SECONDS = 10;
        const int MAX_LOST_KEEPALIVE = 4;
        CloudSocket mCloudSocket;
        Thread mThread;
        bool mThreadRunning = false;
        const int CONNECT_TIMEOUT_SECONDS = 30;
     //   int mLastObjectSendSeconds;
        private Object mLock;

        public CommModel(HCLogger logger)
        {
            mOutstandingKeepAlives = 0;
            mLock = new Object();
            mSendQueue = new Queue<ICommObject>();
            mReceivers = new List<ICommReceiver>();
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
                mLog.SendToHost("CommModel", "Adding object to Queue");
                mSendQueue.Enqueue(obj);
            }

            processSendQueue();
        }
        #endregion

        #region socketListener
        public void OnCloudFrameReceived(Object sender, FrameReceivedArgs e)
        {
           // lock (mLock)
            {
                switch (e.ObjectId)
                {
                    case OBJ_KEEPALIVE:
                        mLog.SendToHost("CommModel", "keepalive received");
                        --mOutstandingKeepAlives;
                        break;
                    case OBJ_SERVERNAME:
                        string serverName = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                        changeState(CommState.Connected);

                        mLog.SendToHost("CommModel", string.Format("Connected with server: {0}", serverName));
                        processSendQueue();
                        break;
                    case OBJ_ROOM_TEMPERATURE:
                        {
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            RoomTemperature temp = new RoomTemperature();
                            temp.deserialise(json);
                            foreach (var receiver in mReceivers)
                            {
                                receiver.receiveObject(temp);
                            }

                        }
                        break;
                    case OBJ_SET_TEMPERATURE:
                        {
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            SetTemperature temp = new SetTemperature();
                            temp.deserialise(json);
                            foreach (var receiver in mReceivers)
                            {
                                receiver.receiveObject(temp);
                            }

                        }
                        break;
                    case OBJ_ROOMLIST:
                        { 
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            RoomList roomList = new RoomList();
                            roomList.deserialise(json);
                            foreach (var receiver in mReceivers)
                            {
                                receiver.receiveObject(roomList);
                            }
                        }

                        break;
                    default:
                        string payload = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                        mLog.SendToHost("CommModel", "Payload received");
                        break;
                }
            }
        }

        public void OnCloudSocketConnected(Object sender, EventArgs e)
        {
           // lock (mLock)
            {
              //  mLastObjectSendSeconds = 0;
                string modelName = Android.OS.Build.Model;
                changeState(mCommState = CommState.NameSend);
                mCloudSocket.sendFrame(OBJ_HCNAME, System.Text.Encoding.ASCII.GetBytes(modelName).ToList());
            }
        }
        #endregion

        private void changeState(CommState newState)
        {
            mLog.SendToHost("CommModel", string.Format("State change, from: {0} to: {1}", mCommState, newState));
            switch (mCommState)
            {
                case CommState.Init:
                    {
                        switch (newState)
                        {
                            case CommState.Disconnected:
                                {
                                    mConnectTimeoutSeconds = RECONNECT_SECONDS;
                                    break;
                                }
                            default:
                                {
                                    mLog.SendToHost("CommModel", string.Format("Unknown state change, from: {0} to: {1}", mCommState, newState));
                                    break;
                                }
                        }
                        break;
                    }
                case CommState.NameSend:
                    {
                        switch (newState)
                        {
                            case CommState.Connected:
                                {
                                    foreach (var receiver in mReceivers)
                                    {
                                        receiver.connected();
                                    }
                                    break;
                                }
                        }
                        break;
                    }
                case CommState.Connected:
                    {
                        switch (newState)
                        {
                            case CommState.Disconnected:
                                {
                                    mConnectTimeoutSeconds = RECONNECT_SECONDS;
                                    foreach (var receiver in mReceivers)
                                    {
                                        receiver.disconnected();
                                    }
                                    if (mCloudSocket != null)
                                    {
                                        mCloudSocket.Dispose();
                                    }
                                    break;
                                }
                        }
                        break;
                    }
                case CommState.Connecting:
                    {

                        break;
                    }
            }

            mCommState = newState;
        }

        private void startConnect()
        {
            mLog.SendToHost("CommModel", "startConnect");
            mOutstandingKeepAlives = 0;

            changeState(CommState.Connecting);
            if (mCloudSocket != null)
            {
                mCloudSocket.Dispose();
            }
            mCloudSocket = new CloudSocket(mLog);
            mCloudSocket.OnFrameReceived += OnCloudFrameReceived;
            mCloudSocket.OnSocketConnected += OnCloudSocketConnected;
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
          //  lock (mLock)
            {
              //  mLastObjectSendSeconds = 0;
                string json = obj.serialise();
                return mCloudSocket.sendFrame(obj.objectId(), System.Text.Encoding.ASCII.GetBytes(json).ToList());
            }
        }

        private void sendKeepAlive()
        {
            if (mCommState == CommState.Connected)
            {
                if (mOutstandingKeepAlives > 0)
                {
                    mLog.SendToHost("CommModem", string.Format("Keepalive missing ({0}), reconnecting", mOutstandingKeepAlives));

                    changeState(CommState.Disconnected);
                }
                else
                {
                    mLog.SendToHost("CommModem", "sending keepalive");
                  //  mLastObjectSendSeconds = 0;
                   // lock (mLock)
                    {
                        mCloudSocket.sendFrame(OBJ_KEEPALIVE, new List<byte>());
                    }
                    ++mOutstandingKeepAlives;
                }
            }
        }

        public void registerCommReceiver(ICommReceiver receiver)
        {
            mLog.SendToHost("CommModel", "registerCommReceiver");
            mReceivers.Add(receiver);
            // Sending the current state of the connection
            if (mCommState == CommState.Connected)
            {
                foreach(var receiverItem in mReceivers)
                {
                    receiverItem.connected();
                }
            }
            else
            {
                foreach (var receiverItem in mReceivers)
                {
                    receiverItem.disconnected();
                }
            }
        }

        public void unRegisterCommReceiver(ICommReceiver receiver)
        {
            mLog.SendToHost("CommModel", "unRegisterCommReceiver");
            mReceivers.Remove(receiver);
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
            const int THREAD_INTERVAL_SECONDS = 1;
            int keepAliveSendSecondsAgo = 0;
            while (mThreadRunning)
            {
                try
                {
                   Thread.Sleep(THREAD_INTERVAL_SECONDS * 1000);
                  //  mLog.SendToHost("CommModel", "Maintenance Thread 1");
                    //   lock (mLock)
                    {

                        if (mCommState == CommState.Disconnected)
                        {
                            mLog.SendToHost("CommModel", "Maintenance Thread 2");
                            --mConnectTimeoutSeconds;
                            if (mConnectTimeoutSeconds <= 0)
                            {
                                startConnect();
                            }
                        }

                        if (mCloudSocket != null)
                        {
                     //       mLog.SendToHost("CommModel", "Maintenance Thread 3");

                            if (mCommState == CommState.Connected && mCloudSocket.isActive())
                            {
                         //       mLog.SendToHost("CommModel", "Maintenance Thread 4");
                                ++keepAliveSendSecondsAgo;
                                if (keepAliveSendSecondsAgo > KEEPALIVE_INTERVAL_SECONDS)
                                {
                                    //           mLog.SendToHost("CommModel", "Maintenance Thread 5");
                                    keepAliveSendSecondsAgo = 0;
                                    sendKeepAlive();
                                }
                            }

                            // No longer active (connected failed, or disconnected
                            if (!mCloudSocket.isActive())
                            {
                                mLog.SendToHost("CommModel", "Cloudsocket no longer active, restarting connection");
                                changeState(CommState.Disconnected);
                            }
                        }
                    }
                }

                catch (Exception ex )
                {
                    mLog.SendToHost("CommModel", "Maintenance thread exception:" + ex.Message);
                }
            }

            mLog.SendToHost("CommModel", "Maintenance Thread Exit thread");
        }
    }
}