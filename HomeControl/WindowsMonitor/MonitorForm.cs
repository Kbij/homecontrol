using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsMonitor.DAL;

namespace WindowsMonitor
{
    public partial class MonitorForm : Form
    {
        GpsLocation mHome;
        Dictionary<string, List<GpsLocation>> mLocations;
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
            mHome = new GpsLocation(51.0536328, 3.6439627, 0, DateTime.Now);
            mDal = new LocationDal();
            cmbHours.Items.Add(new Item("6", 6));
            cmbHours.Items.Add(new Item("12", 12));
            cmbHours.Items.Add(new Item("24", 24));
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

        private void tmrUpdate_Tick(object sender, EventArgs e)
        {
            Item itm = (Item)cmbHours.SelectedItem;
            mLocations = mDal.fillLastLocation(itm.Value);
            Invoke((MethodInvoker)(() => this.updateSummary()));
            if (tmrUpdate.Interval < 1000)
            {
                tmrUpdate.Interval = 30000;
            }
        }

        private void btnKML_Click(object sender, EventArgs e)
        {
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
    }
}
