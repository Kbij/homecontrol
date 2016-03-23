using Android.Util;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Threading;

namespace HomeControl.Comm
{
    public class CommModel: IDisposable
    {
        Thread mThread;
        bool mThreadRunning;
        TcpClient mClient; // Creates a TCP Client
        NetworkStream mStream;
        ICommReceiver mReceiver = null;
        const string HEADER = "HCM";
       // System.Timers.Timer mTimer;
        int mOutstandingKeepAlives;
        enum CommState { Connecting, NameSend, Connected, Disconnected};
        CommState mCommState;
        List<Byte> mBuffer;
        Queue<ICommObject> mSendQueue;
        RemoteLogClient mLog;
        bool mCommStarted;
        const int OBJ_KEEPALIVE = 0;
        const int OBJ_HCNAME = 1;
        const int OBJ_SERVERNAME = 2;

        const int KEEPALIVE_INTERVAL_SECONDS = 30;
        const int MAX_LOST_KEEPALIVE = 4;

        public CommModel()
        {
            mSendQueue = new Queue<ICommObject>();
            mOutstandingKeepAlives = 0;
            mCommState = CommState.Disconnected;
            mBuffer = new List<byte>();
            mLog = new RemoteLogClient("192.168.10.10", 8001);
            mLog.SendToHost("CommModel created");
            mCommStarted = false;
        }

        public void startComm()
        {
            if (!mCommStarted)
            {
                startThread();

                //mTimer = new System.Timers.Timer();
                //mTimer.Interval = KEEPALIVE_INTERVAL_SECONDS * 1000;
                //mTimer.Elapsed += new System.Timers.ElapsedEventHandler(sendKeepAlive);
                //mTimer.Start();
                mCommStarted = true;
                mLog.SendToHost("CommModel started");
            }
        }

        public void Dispose()
        {
            mLog.SendToHost("Dispose");
            stopThread();
        }

        public bool sendObject(ICommObject obj)
        {
            try
            {
                if (mClient != null && mClient.Connected)
                {
                    List<Byte> frame = new List<byte>();
                    frame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                    string json = obj.serialise();
                    int length = json.Length;
                    int msb = (int)(length / 256);
                    int lsb = length - (msb * 256);
                    frame.Add((byte)msb);
                    frame.Add((byte)lsb);
                    frame.Add(obj.objectId());
                    frame.AddRange(System.Text.Encoding.ASCII.GetBytes(json));

                    mStream.Write(frame.ToArray(), 0, frame.Count);
                    return true;
                }
            }
            catch(Exception ex)
            {
                Log.Debug("Comm", string.Format("Exception while sending object:", ex.Message));
            }

            return false;
        }

        public void sendObjectQueued(ICommObject obj)
        {
            lock(mSendQueue)
            {
                mSendQueue.Enqueue(obj);
            }

            processSendQueue();
        }

        private void processSendQueue()
        {
            bool success = true;
            while (success && mSendQueue.Count > 0)
            {
                success = sendObject(mSendQueue.Peek());
                if (success)
                {
                    mSendQueue.Dequeue();
                }
            }
        }

        private void sendName()
        {
            try
            {
                //Sending the model name first
                List<byte> frame = new List<byte>();
                frame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                string modelName = Android.OS.Build.Model;
                int length = modelName.Length;
                int msb = (int)(length / 256);
                int lsb = length - (msb * 256);
                frame.Add((byte)msb);
                frame.Add((byte)lsb);
                frame.Add(OBJ_HCNAME);
                frame.AddRange(System.Text.Encoding.ASCII.GetBytes(modelName));
                mStream.Write(frame.ToArray(), 0, frame.Count);
            }
            catch(Exception ex)
            {
                Log.Debug("CommModel", string.Format("Exception while sending our name: {}", ex.ToString()));
                mClient.Close();
            }
        }

        private void sendKeepAlive(object sender, System.Timers.ElapsedEventArgs e)
        {
            try
            {
                if (mClient != null && mClient.Connected)
                {
                    List<Byte> frame = new List<byte>();
                    frame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                    frame.Add(0);
                    frame.Add(0);
                    frame.Add(OBJ_KEEPALIVE);

                    mStream.Write(frame.ToArray(), 0, frame.Count);
                    ++mOutstandingKeepAlives;
                    if (mOutstandingKeepAlives > MAX_LOST_KEEPALIVE)
                    {
                        Log.Debug("CommModel", "To many outstanding pings, closing connection");

                        mOutstandingKeepAlives = 0;
                        mClient.Close();
                    }
                }
            }
            catch (Exception ex)
            {
                Log.Debug("Comm", string.Format("Exception while sending keepalive:", ex.ToString()));
            }
        }

        public void registerCommReceiver(ICommReceiver receiver)
        {
            mLog.SendToHost("registerCommReceiver");
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
            mLog.SendToHost("unRegisterCommReceiver");
            mReceiver = null;
        }

        private void startThread()
        {
            mThreadRunning = true;
            mThread = new Thread(commThread);
            mThread.Start();
        }

        private void stopThread()
        {
            mThreadRunning = false;
            mThread.Join();
        }

        private void commThread()
        {
            while (mThreadRunning)
            {
                try
                {
                    Log.Debug("CommModel", "Restarting connection");

                    mClient = new TcpClient("paradijs.mooo.com", 5678); //Try to Connect
                    mStream = mClient.GetStream();
                    mLog.SendToHost("Connected");

                    Log.Debug("CommModel", "Connecting... Sending our name");
                    mCommState = CommState.Connecting;
                    sendName();
                    mCommState = CommState.NameSend;

                    int bytesReceived = 0;
                    Byte[] bytes = new Byte[256];
                    while ((bytesReceived = mStream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        mBuffer.AddRange(bytes.Take(bytesReceived).ToArray());
                        processReceiveBuffer();
                    }
                }
                catch (Exception ex)
                {
                    Log.Debug("Comm", string.Format("Exception in comm thread: {0}", ex.Message));
                    mLog.SendToHost(string.Format("Exception in comm thread: {0}", ex.Message));
                }

                mCommState = CommState.Disconnected;
                if (mReceiver != null)
                {
                    mReceiver.disconnected();
                }
                Log.Debug("CommModel", "Disconnected, sleeping 5s...");
                mLog.SendToHost("Disconnected");
                Thread.Sleep(5000);
            }
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
                        switch (objectId)
                        {
                            case OBJ_KEEPALIVE:
                                mOutstandingKeepAlives = 0;
                                break;
                            case OBJ_SERVERNAME:
                                string serverName = System.Text.Encoding.ASCII.GetString(mBuffer.GetRange(HEADER.Length + 3, length).ToArray());
                                if (mCommState == CommState.NameSend)
                                {
                                    if (mReceiver != null)
                                    {
                                        mReceiver.connected();
                                    }
                                }
                                mCommState = CommState.Connected;
                                Log.Debug("CommModel", string.Format("Connected with server: {0}", serverName));

                                processSendQueue();
                                break;
                            default:
                                string payload = System.Text.Encoding.ASCII.GetString(mBuffer.GetRange(HEADER.Length + 3, length).ToArray());
                                break;
                        }

                        mBuffer.RemoveRange(0, length + HEADER.Length + 3);
                    }
                }
            }
        }

    }
}