using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Windows.Forms;
using WindowsMonitor.DAL;
using System.Text;

namespace WindowsMonitor
{
    public partial class MonitorForm : Form
    {
        GpsLocation mHome;
        Dictionary<Tuple<string, DateTime>, List<GpsLocation>> mLocations;
        LocationDal mDal;
        private class Item
        {
            public string Name;
            public int Value;
            public Item(string name, int value)
            {
                Name = name; Value = value;
            }
            public override string ToString()
            {
                // Generates the text shown in the combo box
                return Name;
            }
        }
        public MonitorForm()
        {
            InitializeComponent();
            mLocations = new Dictionary<Tuple<string, DateTime>, List<GpsLocation>>();
            mHome = new GpsLocation(51.0536328, 3.6439627, 0, DateTime.Now);
            mDal = new LocationDal();
            cmbHours.Items.Add(new Item("6", 6));
            cmbHours.Items.Add(new Item("12", 12));
            cmbHours.Items.Add(new Item("24", 24));
            cmbHours.Items.Add(new Item("48", 48));
            cmbHours.Items.Add(new Item("240", 240));
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
                foreach (var cell in mLocations)
                {
                    ListViewItem item = listSummary.Items.Add(cell.Key.Item1);
                    if (cell.Value.Count > 0)
                    {
                        double distance = cell.Value.Last().distance(mHome);
                        string distanceString;
                        if (distance > 1000)
                        {
                            distance = Math.Round((distance / 1000), 2);
                            distanceString = string.Format("{0}km", distance);
                        }
                        else
                        {
                            distance = Math.Round(distance, 2);
                            distanceString = string.Format("{0}m", distance);
                        }
                        item.SubItems.Add(distanceString);
                        item.SubItems.Add(string.Format("{0}", cell.Key.Item2));
                        item.SubItems.Add(string.Format("{0}", cell.Value.Last().TimeStamp));

                        if (cell.Key.Item1 == "Moto G (5)")
                        {
                            this.Text = string.Format("{0}: {1}", distanceString, cell.Value.Last().TimeStamp.ToShortTimeString());
                        }
                    }
                }
            }
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
                        if (client.Key.Item1 == item.Text)
                        {
                            if (client.Value.Count > 0)
                            {
                                string url = string.Format("http://maps.google.com/maps?z=12&t=m&q=loc:{0}+{1}", client.Value.Last().Latitude.ToString(CultureInfo.InvariantCulture), client.Value.Last().Longitude.ToString(CultureInfo.InvariantCulture));
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
                mLocations = mDal.fillLastLocation(itm.Value);
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
                    if (client.Value.Count > 2)
                    {
                        using (System.IO.StreamWriter file = new System.IO.StreamWriter(string.Format("c:\\temp\\{0}.kml", client.Key.Item1)))
                        {
                            file.WriteLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?> <kml xmlns=\"http://earth.google.com/kml/2.0\"><Document>");
                            placeMark(client.Value.First(), string.Format("Start {0}", client.Value.First().TimeStamp.ToString("HH:mm")), file);
                            line(client.Value, file);
                            GpsLocation previousLocation = client.Value.First();
                            foreach (var gpsloc in client.Value)
                            {
                                if (previousLocation.distance(gpsloc) > 500)
                                {
                                    placeMark(gpsloc, string.Format("{0}", gpsloc.TimeStamp.ToString("HH:mm")), file);
                                }
                                previousLocation = gpsloc;
                            }
                            file.WriteLine("</Document></kml>");
                        }
                    }
                }
            }
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
    }
}
