using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Server
{
    public class HCClient
    {
        private TcpClient mClient;
        NetworkStream mStream;
        private string mName;
        Thread mReadThread;
        List<Byte> mBuffer;
        const string HEADER = "HCM";
        ICommListener mCommListener;

        const int OBJ_KEEPALIVE = 0;
        const int OBJ_HCNAME = 1;
        const int OBJ_SERVERNAME = 2;
        const string SERVERNAME = "HCM SERVER";

        public HCClient(TcpClient client, ICommListener commListener)
        {
            mClient = client;
            mCommListener = commListener;
            mBuffer = new List<byte>();
            mStream = mClient.GetStream();
            mName = "Unknown";
            mReadThread = new Thread(readThread);
            mReadThread.Start();
        }

        ~HCClient()
        {
            mClient.Close();
        }

        public string GetName()
        {
            return mName;
        }

        private void readThread()
        {
            // Get a stream object for reading and writing
            Byte[] bytes = new Byte[256];
            int i;

            try {
                // Loop to receive all the data sent by the client.
                while ((i = mStream.Read(bytes, 0, bytes.Length)) != 0)
                {
                    mBuffer.AddRange(bytes.Take(i).ToArray());

                    bool frameFound = true;
                    while (frameFound)
                    {
                        frameFound = false;
                        if (mBuffer.Count > (HEADER.Length + 3))
                        {
                            int length = mBuffer[HEADER.Length] * 256;
                            length += mBuffer[HEADER.Length + 1];
                            byte objectId = mBuffer[HEADER.Length + 2];
                            if (mBuffer.Count >= (length + HEADER.Length + 3))
                            {
                                frameFound = true;
                                string payload = System.Text.Encoding.ASCII.GetString(mBuffer.GetRange(HEADER.Length + 3, length).ToArray());
                               // mCommListener.logMessage(string.Format("Received payload length:{0}", length));

                                switch (objectId)
                                {
                                    case OBJ_KEEPALIVE:
                                        {
                                            List<Byte> frame = new List<byte>();
                                            frame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                                            frame.Add(0);
                                            frame.Add(0);
                                            frame.Add(0);

                                            mStream.Write(frame.ToArray(), 0, frame.Count);
                                            break;
                                        }
                                    case OBJ_HCNAME:
                                        {
                                            mName = payload;
                                            mCommListener.logMessage(string.Format("Connected with: {0}", mName));
                                            //Sending servername
                                            List<Byte> frame = new List<byte>();
                                            frame.AddRange(System.Text.Encoding.ASCII.GetBytes("HCM"));
                                            frame.Add(0);
                                            frame.Add(0);
                                            frame.Add(OBJ_SERVERNAME);
                                            frame.AddRange(System.Text.Encoding.ASCII.GetBytes(SERVERNAME));
                                            frame[System.Text.Encoding.ASCII.GetBytes("HCM").Length + 1] = (byte)SERVERNAME.Length;
                                            mStream.Write(frame.ToArray(), 0, frame.Count);
                                            break;
                                        }
                                    default:
                                        Object receivedObject = ObjectFactory.createObject(objectId, payload);
                                        if (receivedObject != null)
                                        {
                                            mCommListener.receiveObject(mName, receivedObject);
                                        }
                                        break;
                                }

                                mBuffer.RemoveRange(0, length + HEADER.Length + 3);
                            }
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                mCommListener.logMessage(string.Format("Exception in readthread: {0}", ex.Message));
            }
            try { mClient.Close(); } catch (Exception) { }

            mCommListener.logMessage("Connection closed: " + mName);
        }
        
    }
}
