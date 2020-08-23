using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Windows.Forms;
using WindowsMonitor.DAL;
using System.Text;
using System.IO;

namespace WindowsMonitor
{
    public partial class MonitorForm : Form
    {
        GpsLocation mHome;
        Dictionary<string, GpsClient> mLocations;
        LocationDal mDal;
        public MonitorForm()
        {
            InitializeComponent();
            mLocations = new Dictionary<string, GpsClient>();
            mHome = new GpsLocation(51.0536, 3.64393, 0, DateTime.Now);
            mDal = new LocationDal();
            cmbHours.Items.Add(new Item("6", 6));
            cmbHours.Items.Add(new Item("12", 12));
            cmbHours.Items.Add(new Item("24", 24));
            cmbHours.Items.Add(new Item("48", 48));
            cmbHours.Items.Add(new Item("240", 240));
            cmbHours.Items.Add(new Item("1 Month", 24 * 31));
            cmbHours.Items.Add(new Item("6 Months", 24 * 31 * 6));
            cmbHours.SelectedIndex = 0;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            
        }

        private void updateSummary()
        {
            lock (mLocations)
            {
                listSummary.Items.Clear();
                cmbClient.Items.Clear();
                foreach (var cell in mLocations)
                {
                    ListViewItem item = listSummary.Items.Add(cell.Key);
                    if (cell.Value.Locations.Count > 0)
                    {
                        double distance = cell.Value.Locations.Last().distance(mHome);
                        string distanceString;
                        if (distance > 1000)
                        {
                            distance = Math.Round((distance / 1000), 2);
                            distanceString = string.Format("{0:F1}km", distance);
                        }
                        else
                        {
                            distance = Math.Round(distance, 2);
                            distanceString = string.Format("{0:F0}m", distance);
                        }
                        item.SubItems.Add(distanceString);
                        item.SubItems.Add(string.Format("{0}m", cell.Value.Locations.Last().Accuracy));
                        item.SubItems.Add(string.Format("{0}", cell.Value.LastMessage));
                        item.SubItems.Add(string.Format("{0}", cell.Value.Locations.Last().TimeStamp));
                        item.SubItems.Add(string.Format("{0}", cell.Value.LocationInterval));
                        item.SubItems.Add(string.Format("{0}%", cell.Value.BatteryLevel));

                        if (cell.Key == "moto g(8) plus")
                        {
                            this.Text = string.Format("{0}: {1}", distanceString, cell.Value.Locations.Last().TimeStamp.ToShortTimeString());
                        }
                    }

                    cmbClient.Items.Add(cell.Key);
                }
            }
            if (cmbClient.Items.Count > 0) cmbClient.SelectedIndex = 0;
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
                    foreach(var client in mLocations)
                    {
                        if (client.Key == item.Text)
                        {
                            if (client.Value.Locations.Count > 0)
                            {
                                string url = string.Format("http://maps.google.com/maps?z=12&t=m&q=loc:{0}+{1}", client.Value.Locations.Last().Latitude.ToString(CultureInfo.InvariantCulture), client.Value.Locations.Last().Longitude.ToString(CultureInfo.InvariantCulture));
                                ProcessStartInfo sInfo = new ProcessStartInfo(url);
                                Process.Start(sInfo);
                            }

                        }
                    }
                }
            }
        }

        private void refresh()
        {
            lock(mLocations)
            {
                Item itm = (Item)cmbHours.SelectedItem;
                mLocations = mDal.fillLastLocation(itm.Value, "");
                Invoke((MethodInvoker)(() => this.updateSummary()));
            }
        }

        private void tmrUpdate_Tick(object sender, EventArgs e)
        {
            refresh();
            if (tmrUpdate.Interval < 1000)
            {
                tmrUpdate.Interval = 30000;
            }
        }

        private void btnKML_Click(object sender, EventArgs e)
        {
            lock (mLocations)
            {
                foreach (var client in mLocations)
                {
                    if (cmbClient.SelectedItem.ToString() != client.Key) continue;

                    if (client.Value.Locations.Count > 2)
                    {
                        string kmlFileName = string.Format("{0}\\{1}.kml", Path.GetTempPath(), client.Key);
                        using (System.IO.StreamWriter file = new System.IO.StreamWriter(kmlFileName))
                        {
                            file.WriteLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?> <kml xmlns=\"http://earth.google.com/kml/2.0\"><Document>");
                            placeMark(client.Value.Locations.First(), string.Format("Start {0}", client.Value.Locations.First().TimeStamp.ToString("HH:mm")), file);
                            line(client.Value.Locations, file);
                            GpsLocation previousLocation = client.Value.Locations.First();
                            List<GpsLocation> sameLocation = new List<GpsLocation>();
                            foreach (var gpsloc in client.Value.Locations)
                            {
                                if (previousLocation.distance(gpsloc) > 500)
                                {
                                    placeMark(gpsloc, string.Format("{0}", gpsloc.TimeStamp.ToString("HH:mm")), file);
                                }

                                //If no entries, or if current location is near the first of this location
                                if ((sameLocation.Count == 0) && (gpsloc.distance(previousLocation) < 100) || ((sameLocation.Count > 0) && (sameLocation.First().distance(gpsloc) < 100)))
                                {
                                    sameLocation.Add(gpsloc);
                                }
                                else
                                {
                                    if (sameLocation.Count > 2)
                                    {
                                        //Left the location
                                        placeMark(average(sameLocation), string.Format("{0} - {1}", sameLocation.First().TimeStamp.ToString("HH:mm"), sameLocation.Last().TimeStamp.ToString("HH:mm")), file);
                                    }
                                    sameLocation.Clear();
                                }
                                previousLocation = gpsloc;
                            }
                            file.WriteLine("</Document></kml>");
                        }

                        System.Diagnostics.Process.Start(kmlFileName);
                    }
                }
            }
        }

        private GpsLocation average(List<GpsLocation> list)
        {
            if (list.Count > 1)
            {
                double latitudeSum = 0;
                double longitudeSum = 0;
                foreach(var loc in list)
                {
                    latitudeSum += loc.Latitude;
                    longitudeSum += loc.Longitude;
                }
                return new GpsLocation(latitudeSum / list.Count, longitudeSum / list.Count, 0, list.First().TimeStamp);
            }

            return new GpsLocation(0, 0, 0, DateTime.Now);
        }

        private void line(List<GpsLocation> list, System.IO.TextWriter stream)
        {
            if (list.Count < 2) return;

            stream.WriteLine("<Placemark>");
            stream.WriteLine("<LineString><tessellate>1</tessellate>");
            stream.WriteLine("<coordinates>");
            foreach(var point in list)
            {
                stream.WriteLine(string.Format("{0},{1},0", point.Longitude.ToString(CultureInfo.InvariantCulture), point.Latitude.ToString(CultureInfo.InvariantCulture)));
            }
            stream.WriteLine("</coordinates>");
            stream.WriteLine("</LineString>");
            stream.WriteLine("</Placemark>");
        }

        private void cmbHours_SelectionChangeCommitted(object sender, EventArgs e)
        {
            refresh();
        }

        private void placeMark(GpsLocation location, string name, System.IO.TextWriter stream)
        {
            StringBuilder output = new StringBuilder();
            output.Append("<Placemark>");
            output.Append("<name>");
            output.Append(name);
            output.Append("</name>\n");
            output.Append("<Point><coordinates>\n");
            output.Append(string.Format("{0}, {1}, 0\n", location.Longitude.ToString(CultureInfo.InvariantCulture), location.Latitude.ToString(CultureInfo.InvariantCulture)));
            output.Append("</coordinates></Point></Placemark>\n");
            stream.WriteLine(output.ToString());
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            TemperatureViewer viewer = new TemperatureViewer();
            viewer.Show();
        }

        private void MonitorForm_KeyPress(object sender, KeyPressEventArgs e)
        {
        }

        private void MonitorForm_KeyDown(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.F11:
                {
                    if (openFileDialog.ShowDialog() == DialogResult.OK)
                    {
                        LocationViewer location = new LocationViewer(openFileDialog.FileName);
                        location.Show();

                     }
                     break;
                }
                case Keys.F12:
                {
                    LocationViewer location = new LocationViewer();
                    location.Show();
                    break;
                }
                default:
                {
                    break;
                }

            }
        }

        private void mnInterval0_Click(object sender, EventArgs e)
        {
            updateInterval(sender);
        }
        private void mnInterval5_Click(object sender, EventArgs e)
        {
            updateInterval(sender);
        }

        private void mnInterval30_Click(object sender, EventArgs e)
        {
            updateInterval(sender);
        }
        private void updateInterval(object sender)
        {
            if (listSummary.SelectedItems.Count > 0)
            {
                string clientName = listSummary.SelectedItems[0].Text;
                ToolStripMenuItem menuItem = sender as ToolStripMenuItem;
                if (menuItem != null)
                {
                    int interval = 0;
                    if (int.TryParse(menuItem.Text, out interval))
                    {
                        mDal.updateInterval(clientName, interval);
                    }
                }
            }
        }


    }
}
