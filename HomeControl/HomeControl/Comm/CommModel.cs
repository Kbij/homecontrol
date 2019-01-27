using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace HomeControl.Comm
{
    public class CommModel: IDisposable
    {
        HashSet<ICommReceiver> mReceivers;
        enum CommState {Init, Disconnected, Connecting, NameSend, Connected};
        CommState mCommState = CommState.Init;
        int mConnectTimeoutSeconds;
        Queue<ICommObject> mSendQueue;
        HCLogger mLog = null;
        bool mCommStarted;
        int mOutstandingKeepAlives;
        const int OBJ_KEEPALIVE = 0;
        const int OBJ_HCNAME = 1;
        const int OBJ_SERVERNAME = 2;

        const int OBJ_ROOMLIST = 21;
        const int OBJ_ROOM_TEMPERATURE = 22;
        const int OBJ_SET_TEMPERATURE = 23;
        const int OBJ_LOCATION_INTERVAL = 30;

        const int KEEPALIVE_INTERVAL_SECONDS = 30;
        const int RECONNECT_SECONDS = 10;
        const int MAX_LOST_KEEPALIVE = 4;
        CloudSocket mCloudSocket;
        Thread mThread;
        bool mThreadRunning = false;
        const int CONNECT_TIMEOUT_SECONDS = 30;
     //   int mLastObjectSendSeconds;
        private Object mLock;
        Guid mId;
        private static CommModel mInstance = null;
        public static CommModel Instance(HCLogger logger)
        {
            if (mInstance == null)
            {
                mInstance = new CommModel(logger);
                mInstance.startComm();
            }
            return mInstance;
        }

        public static CommModel Instance()
        {
            if (mInstance == null)
            {
                mInstance = new CommModel(null);
                mInstance.startComm();
            }
            return mInstance;
        }

        public CommModel(HCLogger logger)
        {
            mId = Guid.NewGuid();
            mOutstandingKeepAlives = 0;
            mLock = new Object();
            mSendQueue = new Queue<ICommObject>();
            mReceivers = new HashSet<ICommReceiver>();
            mLog = logger;
            if (mLog != null) mLog.SendToHost("CommModel", "CommModel created");
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
                if (mLog != null) mLog.SendToHost("CommModel", "CommModel started");
            }
        }

        public void Dispose()
        {
            stopMaintenanceThread();
            if (mCloudSocket != null)
            {
                if (mLog != null) mLog.SendToHost("CommModel", string.Format("dispose commmodel, disposing socket: {0}", mCloudSocket.Id()));
                mCloudSocket.OnFrameReceived -= OnCloudFrameReceived;
                mCloudSocket.OnSocketConnected -= OnCloudSocketConnected;
                mCloudSocket.Dispose();
                mCloudSocket = null;
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
        public void OnCloudFrameReceived(Object sender, FrameReceivedArgs e)
        {
           // lock (mLock)
            {
                switch (e.ObjectId)
                {
                    case OBJ_KEEPALIVE:
                        if (mLog != null) mLog.SendToHost("CommModel", "keepalive received");
                        --mOutstandingKeepAlives;
                        break;
                    case OBJ_SERVERNAME:
                        string serverName = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                        changeState(CommState.Connected);

                        if (mLog != null) mLog.SendToHost("CommModel", string.Format("Connected with server: {0}", serverName));
                        processSendQueue();
                        break;
                    case OBJ_ROOM_TEMPERATURE:
                        {
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            RoomTemperature temp = new RoomTemperature();
                            temp.deserialise(json);
                            lock (mReceivers)
                            {
                                foreach (var receiver in mReceivers)
                                {
                                    receiver.receiveObject(temp);
                                }
                            }
                        }
                        break;
                    case OBJ_SET_TEMPERATURE:
                        {
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            SetTemperature temp = new SetTemperature();
                            temp.deserialise(json);
                            lock (mReceivers)
                            {
                                foreach (var receiver in mReceivers)
                                {
                                    receiver.receiveObject(temp);
                                }
                            }
                        }
                        break;
                    case OBJ_ROOMLIST:
                        { 
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            RoomList roomList = new RoomList();
                            roomList.deserialise(json);
                            lock (mReceivers)
                            {
                                foreach (var receiver in mReceivers)
                                {
                                    receiver.receiveObject(roomList);
                                }
                            }
                        }

                        break;
                    case OBJ_LOCATION_INTERVAL:
                        {
                            string json = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                            LocationInterval locationInterval = new LocationInterval();
                            locationInterval.deserialise(json);
                            lock (mReceivers)
                            {
                                foreach (var receiver in mReceivers)
                                {
                                    receiver.receiveObject(locationInterval);
                                }
                            }
                        }

                        break;
                    default:
                        string payload = System.Text.Encoding.ASCII.GetString(e.Frame.ToArray());
                        if (mLog != null) mLog.SendToHost("CommModel", "Payload received");
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
            if (mLog != null) mLog.SendToHost("CommModel", string.Format("State change, from: {0} to: {1}", mCommState, newState));
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
                                    if (mLog != null) mLog.SendToHost("CommModel", string.Format("Unknown state change, from: {0} to: {1}", mCommState, newState));
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
                                    lock (mReceivers)
                                    {

                                        foreach (var receiver in mReceivers)
                                        {
                                            receiver.connected();
                                        }
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
                                    lock (mReceivers)
                                    {
                                        foreach (var receiver in mReceivers)
                                        {
                                            receiver.disconnected();
                                        }
                                    }
                                    if (mCloudSocket != null)
                                    {
                                        if (mLog != null) mLog.SendToHost("CommModel", string.Format("disconnected, disposing socket: {0}", mCloudSocket.Id()));
                                        mCloudSocket.OnFrameReceived -= OnCloudFrameReceived;
                                        mCloudSocket.OnSocketConnected -= OnCloudSocketConnected;
                                        mCloudSocket.Dispose();
                                        mCloudSocket = null;
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
            if (mLog != null) mLog.SendToHost("CommModel", "startConnect");
            mOutstandingKeepAlives = 0;

            changeState(CommState.Connecting);
            if (mCloudSocket != null)
            {
                if (mLog != null) mLog.SendToHost("CommModel", string.Format("startConnect, disposing socket: {0}", mCloudSocket.Id()));
                mCloudSocket.OnFrameReceived -= OnCloudFrameReceived;
                mCloudSocket.OnSocketConnected -= OnCloudSocketConnected;
                mCloudSocket.Dispose();
                mCloudSocket = null;
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
                            if (mLog != null) mLog.SendToHost("CommModel", "Object not send, not removed from queue");
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
                    if (mLog != null) mLog.SendToHost("CommModem", string.Format("Keepalive missing ({0}), reconnecting", mOutstandingKeepAlives));

                    changeState(CommState.Disconnected);
                }
                else
                {
                    if (mLog != null) mLog.SendToHost("CommModem", "sending keepalive");
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
            if (mLog != null) mLog.SendToHost("CommModel", "registerCommReceiver");
            lock (mReceivers)
            {
                mReceivers.Add(receiver);
                // Sending the current state of the connection
                if (mCommState == CommState.Connected)
                {
                    foreach (var receiverItem in mReceivers)
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
        }

        public void unRegisterCommReceiver(ICommReceiver receiver)
        {
            if (mLog != null) mLog.SendToHost("CommModel", "unRegisterCommReceiver");
            lock (mReceivers)
            {
                mReceivers.Remove(receiver);
            }
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
                    //   lock (mLock)
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
                                ++keepAliveSendSecondsAgo;
                                if (keepAliveSendSecondsAgo > KEEPALIVE_INTERVAL_SECONDS)
                                {
                                    keepAliveSendSecondsAgo = 0;
                                    sendKeepAlive();
                                }
                            }

                            // No longer active (connected failed, or disconnected
                            if (!mCloudSocket.isActive())
                            {
                                if (mLog != null) mLog.SendToHost("CommModel", "Cloudsocket no longer active, restarting connection");
                                changeState(CommState.Disconnected);
                            }
                        }
                    }
                }

                catch (Exception ex )
                {
                    if (mLog != null) mLog.SendToHost("CommModel", "Maintenance thread exception:" + ex.Message);
                }
            }

            if (mLog != null) mLog.SendToHost("CommModel", "Maintenance Thread Exit thread");
        }
    }
}