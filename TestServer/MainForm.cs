                                                                                                                                                                                                                                                                                                         using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
//using System.Windows.da
using System.Runtime.Serialization.Json;
using System.IO;
using System.Diagnostics;
using System.Globalization;

namespace Server
{
    public partial class MainForm : Form, ICommListener
    {
        Thread mServerThread;
        bool mThreadRunning = false;
        TcpListener mServer = null;
        //const string HEADER = "HCM";
        GpsLocation mHome;
        Dictionary<string, List<GpsLocation>> mLocations;
        public MainForm()
        {
            InitializeComponent();
            writeLog("Started");
            mHome = new GpsLocation();
            mHome.Latitude = 51.0536328;
            mHome.Longitude = 3.6439627;
            mLocations = new Dictionary<string, List<GpsLocation>>();
            
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            startThread();
        }

        #region logging
        public void receiveObject(string name, Object obj)
        {
            if (obj.GetType() == typeof(GpsLocation))
            {
                GpsLocation location = (GpsLocation)obj;

                lock (mLocations)
                {
                    if (!mLocations.ContainsKey(name))
                    {
                        mLocations[name] = new List<GpsLocation>();
                    }
                    mLocations[name].Add(location);
                }
                logMessage(string.Format("Name: {0}: {1}, distance: {2}", name, location.toString(), location.distance(mHome)));
                Invoke((MethodInvoker)(() => this.updateSummary()));
            }

            if (obj.GetType() == typeof(MessageObject))
            {
                MessageObject message = (MessageObject)obj;
                logMessage(string.Format("Name: {0}: {1}", name, message.toString()));
            }
        }

        private void updateSummary()
        {
            lock (mLocations)
            {
                listSummary.Items.Clear();
                foreach (var cell in mLocations)
                {
                    ListViewItem item = listSummary.Items.Add(cell.Key);
                    if (cell.Value.Count > 0)
                    {
                        double distance = cell.Value.Last().distance(mHome);
                        if (distance > 1000)
                        {
                            distance = Math.Round((distance / 1000), 2);
                            item.SubItems.Add(string.Format("{0}km", distance));
                        }
                        else
                        {
                            distance = Math.Round(distance, 2);
                            item.SubItems.Add(string.Format("{0}m", distance));
                        }
                        item.SubItems.Add(string.Format("{0}", cell.Value.Last().TimeStamp));
                    }
                }
            }
        }

        public void logMessage(string message)
        {
            if (txtLog.InvokeRequired)
            {
                Invoke((MethodInvoker)(() => txtLog.Text += message + "\r\n"));
            }
            else
            {
                txtLog.Text += message + "\r\n";
            }
        }

        private void writeLog(string message)
        {
            if (txtLog.InvokeRequired)
            {
                Invoke((MethodInvoker)(() => txtLog.Text += message + "\r\n"));
            }
            else
            {
                txtLog.Text += message + "\r\n";
            }
        }
        #endregion

        #region thread
        private void startThread()
        {
            mThreadRunning = true;
            mServerThread = new Thread(serverThread);
            mServerThread.Start();
        }

        private void stopThread()
        {
            mThreadRunning = false;
            if (mServerThread != null)
            {
                mServerThread.Join();
            }
        }
        #endregion

        private void serverThread()
        {
            writeLog("Starting listener");
            IPAddress ipAddress = new IPAddress(new byte[] { 0, 0, 0, 0 });
            IPEndPoint localEndPoint = new IPEndPoint(ipAddress, 5678);

            mServer = new TcpListener(localEndPoint);
            mServer.Start();
            // Buffer for reading data
            Byte[] bytes = new Byte[256];
            List<Byte> buffer = new List<byte>();

            try
            {
                while (mThreadRunning)
                {
                    writeLog("Waiting for a connection... ");

                    TcpClient client = mServer.AcceptTcpClient();
                    writeLog("Connected!");
                    HCClient hcClient = new HCClient(client, this);
                }
            }
            catch (Exception ex)
            {
                //writeLog("Exception: " + ex.Message);
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (mServer != null)
            {
                mServer.Stop();
            }
            stopThread();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            txtLog.Clear();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //string header = @"<Placemark>
            //<name>Track</name>
            //<description>

            //</description>
            //<styleUrl>#msn_ylw-pushpin_copy1</styleUrl>
            //<LineString>
            //<tessellate>1</tessellate>
            //<coordinates>";
            string foorter = @"</coordinates></LineString></Placemark>";

            lock (mLocations)
            {
                foreach (var client in mLocations)
                {
                    using (System.IO.StreamWriter file = new System.IO.StreamWriter(string.Format("c:\\temp\\{0}.kml", client.Key)))
                    {
                        file.Write(header(client.Key));
                        foreach (var gpsloc in client.Value)
                        {
                            file.WriteLine(string.Format("{0},{1},0", gpsloc.Longitude.ToString(CultureInfo.InvariantCulture), gpsloc.Latitude.ToString(CultureInfo.InvariantCulture)));
                        }
                        file.Write(foorter);
                    }
                }
            }
        }
        private string header(string name)
        {
            return string.Format("<Placemark><name>{0}</name><description></description><styleUrl>#msn_ylw-pushpin_copy1</styleUrl><LineString><tessellate>1</tessellate><coordinates>", name);
        }

        private void listSummary_DoubleClick(object sender, EventArgs e)
        {
            ListView listView = sender as ListView;

            if (listView.SelectedItems.Count == 1)
            {
                ListView.SelectedListViewItemCollection items = listView.SelectedItems;

                ListViewItem item = items[0];
                lock (mLocations)
                {
                    if (mLocations.ContainsKey(item.Text))
                    {
                        if (mLocations[item.Text].Count > 0)
                        {
                            string url = string.Format("http://maps.google.com/maps?z=12&t=m&q=loc:{0}+{1}", mLocations[item.Text].Last().Latitude.ToString(CultureInfo.InvariantCulture), mLocations[item.Text].Last().Longitude.ToString(CultureInfo.InvariantCulture));
                            ProcessStartInfo sInfo = new ProcessStartInfo(url);
                            Process.Start(sInfo);
                        }
                    }
                }
            }
        }

        private void btnClearKML_Click(object sender, EventArgs e)
        {
            lock (mLocations)
            {
                mLocations.Clear();
            }
        }
    }
}
