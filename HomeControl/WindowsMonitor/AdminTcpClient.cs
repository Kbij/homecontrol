/*
 * AdminTcpClient.cs
 *
 * Windows counterpart of the Android app's AdminTcpClient.kt (see
 * D:\Code\HARemote\app\src\main\java\com\koen\haremote\network\tcp\AdminTcpClient.kt). A small,
 * independent persistent TCP connection to the HomeControl server, used only for the admin
 * live-location feed shown by AdminMapViewer (F11) - completely separate from this app's existing
 * direct-to-MySQL LocationDal, which is left untouched (still used for the F12 view and, here,
 * just to populate the client picker).
 *
 * Wire protocol (mirrors ClientSocket.cpp / HcmProtocol.kt - do not renumber these ids, they are
 * shared with the C++ server and the Android app):
 *
 *   byte 0-2  "HCM"              fixed 3-byte header
 *   byte 3-4  length             payload length, uint16 big-endian (MSB first)
 *   byte 5    objectId           uint8
 *   byte 6..  payload            usually a small ASCII JSON document
 *
 * Not admin-PIN-gated on this side: the server checks the *connecting client's own name*
 * against Client.adminCode on every LocationHistoryRequest (capability-gated, not
 * session-gated - see the C++ server's AdminController and the "Security-model" section of the
 * project notes). So this class just connects under a fixed configured name (App.config's
 * AdminClientName) and sends requests directly, without ever sending an AdminAuthRequest or
 * asking for a PIN. If that name has no adminCode set in the Client table, the server replies
 * with AdminAuthResult(success=false) instead of a LocationHistoryResponse - see AuthDenied.
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Web.Script.Serialization;

namespace WindowsMonitor
{
    public class AdminLocationPoint
    {
        public double Latitude;
        public double Longitude;
        public DateTime TimeStamp;
    }

    public class AdminGeofenceInfo
    {
        public double Latitude;
        public double Longitude;
        public double RadiusMeters;
        public DateTime CreatedAt;
        public DateTime UpdatedAt;
    }

    public class AdminLocationHistory
    {
        public string Client;
        public List<AdminLocationPoint> Points = new List<AdminLocationPoint>();
        public DateTime? LastConnection;
        public AdminGeofenceInfo Geofence;
    }

    public class AdminTcpClient
    {
        // Fired from a background thread - subscribers must marshal back to the UI thread
        // themselves (see AdminMapViewer's InvokeRequired guards), same contract as the Android
        // Listener interface this mirrors.
        public event Action<bool> ConnectionStateChanged;
        public event Action<AdminLocationHistory> LocationHistoryReceived;
        public event Action AuthDenied;

        private static readonly byte[] HeaderBytes = Encoding.ASCII.GetBytes("HCM");
        private const int HEADER_TOTAL_LENGTH = 6; // "HCM" (3) + length (2) + objectId (1)

        private const int OBJ_KEEPALIVE = 0;
        private const int OBJ_HCNAME = 1;
        private const int OBJ_SERVERNAME = 2;
        private const int OBJ_ADMIN_AUTH_RESULT = 41;
        private const int OBJ_LOCATION_HISTORY_REQUEST = 43;
        private const int OBJ_LOCATION_HISTORY_RESPONSE = 44;

        private const int CONNECT_TIMEOUT_MS = 10000;
        // Clients are expected to send a keepalive every ~30s (see the C++ server's Client.cpp
        // comment on RECEIVE_TIMEOUT_MS); this doubles as our socket read timeout so a silent,
        // half-open connection gets noticed and reconnected instead of hanging forever.
        private const int KEEPALIVE_INTERVAL_MS = 30000;
        private const int RECONNECT_DELAY_MS = 15000;

        private readonly string mClientName;
        private string mHost = "";
        private int mPort;
        private volatile bool mRunning;
        private volatile bool mConnected;
        private volatile NetworkStream mStream;
        private readonly object mWriteLock = new object();
        private readonly object mSleepLock = new object();
        private Thread mThread;

        private readonly object mPendingLock = new object();
        private string mPendingClient;
        private int mPendingMinutes;

        public AdminTcpClient(string clientName)
        {
            mClientName = clientName;
        }

        public bool IsConnected
        {
            get { return mConnected; }
        }

        public void Start(string host, int port)
        {
            mHost = host;
            mPort = port;
            if (mRunning) return;
            mRunning = true;
            mThread = new Thread(RunLoop);
            mThread.IsBackground = true;
            mThread.Name = "AdminTcpClient";
            mThread.Start();
        }

        public void Stop()
        {
            mRunning = false;
            CloseCurrentStream();
            lock (mSleepLock)
            {
                Monitor.PulseAll(mSleepLock);
            }
        }

        /// <summary>
        /// Records what to request next and sends it immediately if already connected (it is
        /// re-sent automatically as soon as a dropped connection comes back up). Callers should
        /// clear whatever history they are currently displaying *before* calling this - mirrors
        /// the 10 sept 2026 HaRemoteViewModel.requestLocationHistory bugfix on the Android side:
        /// without that, switching clients would keep showing the previous client's marker/route/
        /// geofence under the newly selected name until the fresh response arrives.
        /// </summary>
        public void RequestLocationHistory(string client, int minutes)
        {
            lock (mPendingLock)
            {
                mPendingClient = client;
                mPendingMinutes = minutes;
            }
            TrySendPendingRequest();
        }

        private void TrySendPendingRequest()
        {
            string client;
            int minutes;
            lock (mPendingLock)
            {
                client = mPendingClient;
                minutes = mPendingMinutes;
            }
            if (string.IsNullOrEmpty(client)) return;

            var payload = new Dictionary<string, object>();
            payload["client"] = client;
            payload["minutes"] = minutes;
            string json = new JavaScriptSerializer().Serialize(payload);
            SendFrame(OBJ_LOCATION_HISTORY_REQUEST, Encoding.ASCII.GetBytes(json));
        }

        private void RunLoop()
        {
            while (mRunning)
            {
                try
                {
                    ConnectAndPump();
                }
                catch (Exception)
                {
                    // Fall through to reconnect below - same "log and retry" shape as the
                    // Android/C++ counterparts, there is nothing actionable to do here per-attempt.
                }
                SetConnected(false);
                if (!mRunning) return;
                lock (mSleepLock)
                {
                    Monitor.Wait(mSleepLock, RECONNECT_DELAY_MS);
                }
            }
        }

        private void ConnectAndPump()
        {
            string targetHost = mHost;
            int targetPort = mPort;
            if (string.IsNullOrEmpty(targetHost) || targetPort <= 0) return;

            using (TcpClient client = new TcpClient())
            {
                client.NoDelay = true;
                IAsyncResult connectResult = client.BeginConnect(targetHost, targetPort, null, null);
                if (!connectResult.AsyncWaitHandle.WaitOne(CONNECT_TIMEOUT_MS))
                {
                    throw new SocketException((int)SocketError.TimedOut);
                }
                client.EndConnect(connectResult);
                client.ReceiveTimeout = KEEPALIVE_INTERVAL_MS;

                NetworkStream stream = client.GetStream();
                mStream = stream;

                // Must happen immediately: the server only allows ~1s in the "connecting" state
                // before it drops the socket (see the C++ Client::isInactive/CONNECTING_TIMEOUT_MS).
                WriteFrame(stream, OBJ_HCNAME, Encoding.ASCII.GetBytes(mClientName));

                List<byte> receiveBuffer = new List<byte>();
                byte[] readChunk = new byte[1024];
                bool outstandingKeepAlive = false;

                try
                {
                    while (mRunning && mStream == stream)
                    {
                        int bytesRead;
                        try
                        {
                            bytesRead = stream.Read(readChunk, 0, readChunk.Length);
                        }
                        catch (IOException ioEx)
                        {
                            SocketException socketEx = ioEx.InnerException as SocketException;
                            if (socketEx != null && socketEx.SocketErrorCode == SocketError.TimedOut)
                            {
                                if (mConnected)
                                {
                                    if (outstandingKeepAlive)
                                    {
                                        return; // No ack in time; let the outer loop reconnect.
                                    }
                                    WriteFrame(stream, OBJ_KEEPALIVE, new byte[0]);
                                    outstandingKeepAlive = true;
                                }
                                continue;
                            }
                            throw;
                        }

                        if (bytesRead <= 0) return; // Server closed the connection.

                        for (int i = 0; i < bytesRead; i++)
                        {
                            receiveBuffer.Add(readChunk[i]);
                        }

                        List<int> objectIds;
                        List<byte[]> payloads;
                        receiveBuffer = ExtractFrames(receiveBuffer, out objectIds, out payloads);

                        for (int i = 0; i < objectIds.Count; i++)
                        {
                            int objectId = objectIds[i];
                            byte[] payload = payloads[i];
                            switch (objectId)
                            {
                                case OBJ_SERVERNAME:
                                    SetConnected(true);
                                    TrySendPendingRequest();
                                    break;
                                case OBJ_KEEPALIVE:
                                    outstandingKeepAlive = false;
                                    break;
                                case OBJ_ADMIN_AUTH_RESULT:
                                    HandleAuthResult(payload);
                                    break;
                                case OBJ_LOCATION_HISTORY_RESPONSE:
                                    HandleLocationHistoryResponse(payload);
                                    break;
                                default:
                                    break; // Not relevant to the admin feed.
                            }
                        }
                    }
                }
                finally
                {
                    if (mStream == stream) mStream = null;
                }
            }
        }

        private List<byte> ExtractFrames(List<byte> buffer, out List<int> objectIds, out List<byte[]> payloads)
        {
            objectIds = new List<int>();
            payloads = new List<byte[]>();

            while (true)
            {
                int headerIndex = IndexOfHeader(buffer);
                if (headerIndex < 0)
                {
                    // Keep only the last (headerLen - 1) bytes in case they're the start of a
                    // header split across two reads; drop the rest so a garbage stream (or a
                    // stray byte) can't grow the buffer forever - mirrors the C++/Kotlin decoders.
                    int keep = HeaderBytes.Length - 1;
                    if (buffer.Count > keep)
                    {
                        buffer = buffer.GetRange(buffer.Count - keep, keep);
                    }
                    return buffer;
                }
                if (headerIndex > 0)
                {
                    buffer = buffer.GetRange(headerIndex, buffer.Count - headerIndex);
                }
                if (buffer.Count < HEADER_TOTAL_LENGTH) return buffer;

                int dataLength = (buffer[3] << 8) | buffer[4];
                int objectId = buffer[5];
                int totalLength = HEADER_TOTAL_LENGTH + dataLength;
                if (buffer.Count < totalLength) return buffer;

                byte[] payload = buffer.GetRange(HEADER_TOTAL_LENGTH, dataLength).ToArray();
                objectIds.Add(objectId);
                payloads.Add(payload);
                buffer = buffer.GetRange(totalLength, buffer.Count - totalLength);
            }
        }

        private int IndexOfHeader(List<byte> data)
        {
            int headerLength = HeaderBytes.Length;
            if (data.Count < headerLength) return -1;
            for (int i = 0; i <= data.Count - headerLength; i++)
            {
                bool match = true;
                for (int j = 0; j < headerLength; j++)
                {
                    if (data[i + j] != HeaderBytes[j]) { match = false; break; }
                }
                if (match) return i;
            }
            return -1;
        }

        private void WriteFrame(NetworkStream stream, int objectId, byte[] payload)
        {
            lock (mWriteLock)
            {
                int length = payload.Length;
                byte[] frame = new byte[HEADER_TOTAL_LENGTH + length];
                Array.Copy(HeaderBytes, frame, HeaderBytes.Length);
                frame[3] = (byte)((length >> 8) & 0xFF);
                frame[4] = (byte)(length & 0xFF);
                frame[5] = (byte)(objectId & 0xFF);
                Array.Copy(payload, 0, frame, HEADER_TOTAL_LENGTH, length);
                stream.Write(frame, 0, frame.Length);
                stream.Flush();
            }
        }

        /// <summary>
        /// Queues a write on the thread pool instead of writing directly on the caller's thread.
        /// Not strictly required here (WinForms has no NetworkOnMainThreadException), but this
        /// mirrors the lesson from the 7 sept 2026 Android crash fix documented in the project
        /// notes: a blocking socket write should never be assumed safe on whatever thread happens
        /// to call in (RequestLocationHistory is normally called from a ComboBox event on the UI
        /// thread here).
        /// </summary>
        private void SendFrame(int objectId, byte[] payload)
        {
            NetworkStream stream = mStream;
            if (stream == null || !mConnected) return;
            ThreadPool.QueueUserWorkItem(delegate
            {
                try
                {
                    WriteFrame(stream, objectId, payload);
                }
                catch (Exception)
                {
                    // The read loop will notice the dead connection and reconnect.
                }
            });
        }

        private void HandleAuthResult(byte[] payload)
        {
            try
            {
                string json = Encoding.ASCII.GetString(payload);
                Dictionary<string, object> dict = new JavaScriptSerializer().Deserialize<Dictionary<string, object>>(json);
                bool success = dict.ContainsKey("success") && Convert.ToBoolean(dict["success"]);
                if (!success)
                {
                    Action handler = AuthDenied;
                    if (handler != null) handler();
                }
            }
            catch (Exception)
            {
                // Malformed/unexpected payload; nothing actionable, just drop it.
            }
        }

        private void HandleLocationHistoryResponse(byte[] payload)
        {
            try
            {
                string json = Encoding.ASCII.GetString(payload);
                Dictionary<string, object> dict = new JavaScriptSerializer().Deserialize<Dictionary<string, object>>(json);

                AdminLocationHistory history = new AdminLocationHistory();
                object clientObj;
                history.Client = dict.TryGetValue("client", out clientObj) && clientObj != null ? clientObj.ToString() : "";

                object pointsObj;
                if (dict.TryGetValue("points", out pointsObj) && pointsObj is IEnumerable)
                {
                    foreach (object item in (IEnumerable)pointsObj)
                    {
                        IDictionary<string, object> p = item as IDictionary<string, object>;
                        if (p == null) continue;
                        AdminLocationPoint point = new AdminLocationPoint();
                        point.Latitude = Convert.ToDouble(p["lat"]);
                        point.Longitude = Convert.ToDouble(p["lon"]);
                        point.TimeStamp = EpochMillisToLocal(Convert.ToInt64(p["timestamp"]));
                        history.Points.Add(point);
                    }
                }

                object lastConnObj;
                if (dict.TryGetValue("lastConnection", out lastConnObj) && lastConnObj != null)
                {
                    long millis = Convert.ToInt64(lastConnObj);
                    history.LastConnection = millis > 0 ? (DateTime?)EpochMillisToLocal(millis) : null;
                }

                object geofenceObj;
                if (dict.TryGetValue("geofence", out geofenceObj) && geofenceObj is IDictionary<string, object>)
                {
                    IDictionary<string, object> g = (IDictionary<string, object>)geofenceObj;
                    AdminGeofenceInfo geofence = new AdminGeofenceInfo();
                    geofence.Latitude = Convert.ToDouble(g["lat"]);
                    geofence.Longitude = Convert.ToDouble(g["lon"]);
                    geofence.RadiusMeters = Convert.ToDouble(g["radius"]);
                    geofence.CreatedAt = EpochMillisToLocal(Convert.ToInt64(g["createdAt"]));
                    geofence.UpdatedAt = EpochMillisToLocal(Convert.ToInt64(g["updatedAt"]));
                    history.Geofence = geofence;
                }

                Action<AdminLocationHistory> received = LocationHistoryReceived;
                if (received != null) received(history);
            }
            catch (Exception)
            {
                // Malformed/unexpected payload; nothing actionable, just drop it - same
                // best-effort handling as the auth-result parsing above.
            }
        }

        private static readonly DateTime UnixEpoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

        private static DateTime EpochMillisToLocal(long millis)
        {
            return UnixEpoch.AddMilliseconds(millis).ToLocalTime();
        }

        private void SetConnected(bool value)
        {
            if (mConnected != value)
            {
                mConnected = value;
                Action<bool> handler = ConnectionStateChanged;
                if (handler != null) handler(value);
            }
        }

        private void CloseCurrentStream()
        {
            NetworkStream stream = mStream;
            mStream = null;
            if (stream != null)
            {
                try { stream.Close(); }
                catch (Exception) { }
            }
        }
    }
}
