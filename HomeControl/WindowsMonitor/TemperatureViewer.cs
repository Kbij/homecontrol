using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsMonitor.DAL;
using ZedGraph;
using System.Drawing;

namespace WindowsMonitor
{
    public partial class TemperatureViewer : Form
    {
        GraphPane mGraphPane;
        readonly List<Color> GraphColors = new List<Color> {Color.Red, Color.Green, Color.Lavender, Color.Blue, Color.Brown};

        Dictionary<string, HomeControlData.TemperatureDataTable> mTemperatures;
        Dictionary<string, Dictionary<DateTime, int>> mTemperatureCount;

        public TemperatureViewer()
        {
            InitializeComponent();
            mTemperatures = new Dictionary<string, HomeControlData.TemperatureDataTable>();

            // set your pane
            mGraphPane = zedGraph.GraphPane;
            mGraphPane.Title.Text = "Kamer Temperatuur";
            mGraphPane.XAxis.Title.Text = "Tijd";
            mGraphPane.YAxis.Title.Text = "Temperatuur";
            mGraphPane.XAxis.Type = AxisType.Date;
            mGraphPane.Legend.IsVisible = true;
            dtFrom.Value = dtFrom.Value.AddDays(-1);
            lstInterval.SelectedIndex = 0;
            mTemperatureCount = new Dictionary<string, Dictionary<DateTime, int>>();
            refreshRooms();
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            refreshRooms();
        }

        private void refreshRooms()
        {
            HomeControlData.RoomDataTable dt = new HomeControlData.RoomDataTable();
            TemperatureDal dal = new TemperatureDal();

            dal.fillRooms(dt);
            lstRooms.Items.Clear();
            mTemperatures.Clear();

            foreach (var room in dt)
            {
                ListViewItem item = new ListViewItem(room.RoomId);
                lstRooms.Items.Add(item);
            }
        }

        private void lstRooms_ItemChecked(object sender, ItemCheckedEventArgs e)
        {
            refreshData();
        }

        private void refreshData()
        {
            if (!chbQuality.Checked)
            {
                //Check the ones no longer selected in the list
                List<string> removeTemperatures = new List<string>();
                foreach (var tempList in mTemperatures)
                {
                    bool found = false;
                    foreach (ListViewItem item in lstRooms.Items)
                    {
                        if (item.Checked && item.Text == tempList.Key)
                        {
                            found = true;
                        }
                    }
                    if (!found)
                    {
                        removeTemperatures.Add(tempList.Key);
                    }
                }

                foreach (var remove in removeTemperatures)
                {
                    mTemperatures.Remove(remove);
                }

                //Add the new ones to the list
                foreach (ListViewItem item in lstRooms.Items)
                {
                    if (item.Checked)
                    {
                        if (!mTemperatures.ContainsKey(item.ToString()))
                        {
                            HomeControlData.TemperatureDataTable dt = new HomeControlData.TemperatureDataTable();
                            TemperatureDal dal = new TemperatureDal();
                            DateTime start = dtFrom.Value;
                            DateTime end = dtTo.Value;

                            dal.fillTemperatures(dt, start, end, item.Text);
                            mTemperatures[item.Text] = dt;
                            Debug.WriteLine(string.Format("RoomId: {0}, items: {1}", item.Text, dt.Count));
                        }
                    }
                }
            }
            else
            {

                mTemperatureCount = new Dictionary<string, Dictionary<DateTime, int>>();

                //Add the new ones to the list
                foreach (ListViewItem item in lstRooms.Items)
                {
                    if (item.Checked)
                    {
                        if (!mTemperatureCount.ContainsKey(item.ToString()))
                        {
                            HomeControlData.TemperatureDataTable dt = new HomeControlData.TemperatureDataTable();
                            TemperatureDal dal = new TemperatureDal();
                            DateTime start = dtFrom.Value;
                            DateTime end = dtTo.Value;

                            dal.fillTemperatures(dt, start, end, item.Text);
                            foreach(HomeControlData.TemperatureRow temperature in dt.Rows)
                            {
                                if (!mTemperatureCount.ContainsKey(item.Text))
                                {
                                    mTemperatureCount[item.Text] = new Dictionary<DateTime, int>();
                                }
                                if (!mTemperatureCount[item.Text].ContainsKey(dateInterval(temperature.date)))
                                {
                                    mTemperatureCount[item.Text][dateInterval(temperature.date)] = 1;
                                }
                                else
                                {
                                    mTemperatureCount[item.Text][dateInterval(temperature.date)] += 1;
                                }
                            }

                            Debug.WriteLine(string.Format("RoomId: {0}, items: {1}", item.Text, dt.Count));
                        }
                    }
                }
            }

            refreshGrids();
        }

        private DateTime dateInterval(DateTime date)
        {
            TimeSpan interval = TimeSpan.FromMinutes(int.Parse(lstInterval.SelectedItem.ToString()));
            var modTicks = date.Ticks % interval.Ticks;
            var delta = modTicks != 0 ? interval.Ticks - modTicks : 0;

            Debug.WriteLine(string.Format("Orig: {0}, Round: {1}", date, new DateTime(date.Ticks + delta, date.Kind)));
            return new DateTime(date.Ticks + delta, date.Kind);
        }

        private void refreshGrids()
        {
            mGraphPane.CurveList.Clear();
            int colorPos = 0;

            if (!chbQuality.Checked)
            {
                foreach (var temperatues in mTemperatures)
                {
                    PointPairList rawPoints = new PointPairList();
                    PointPairList runninigAveragePoints = new PointPairList();
                    PointPairList runninigAveragePoints2 = new PointPairList();
                    PointPairList weightedAveragePoints = new PointPairList();
                    PointPairList expAveragePoints = new PointPairList();


                    RunningAverageFilter runningAverage = new RunningAverageFilter(10);
                    RunningAverageFilter2 runningAverage2 = new RunningAverageFilter2(12);
                    WeigtedAverageFilter weightedAverage = new WeigtedAverageFilter(new List<double> { 0.02375, 0.00125, 0.025, 0.05, 0.1, 0.2, 0.6 });
                    ExponentialWeightedAverage expAverage = new ExponentialWeightedAverage(0.2);

                    foreach (HomeControlData.TemperatureRow temp in temperatues.Value)
                    {
                        rawPoints.Add(new XDate(temp.date.ToOADate()), (double)temp.temperature);
                        runninigAveragePoints.Add(new XDate(temp.date.ToOADate()), runningAverage.AddValue((double)temp.temperature));
                        runninigAveragePoints2.Add(new XDate(temp.date.ToOADate()), runningAverage2.AddValue((double)temp.temperature));
                        weightedAveragePoints.Add(new XDate(temp.date.ToOADate()), weightedAverage.AddValue((double)temp.temperature));
                        expAveragePoints.Add(new XDate(temp.date.ToOADate()), expAverage.AddValue((double)temp.temperature));
                    }


                    mGraphPane.AddCurve(temperatues.Key, rawPoints, colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black, SymbolType.None);
                    //       mGraphPane.AddCurve(temperatues.Key + "_RunAvg", runninigAveragePoints, colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black, SymbolType.None);
                    //      mGraphPane.AddCurve(temperatues.Key + "_RunAvg2", runninigAveragePoints2, colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black, SymbolType.None);
                    //     mGraphPane.AddCurve(temperatues.Key + "_Weight", weightedAveragePoints, colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black, SymbolType.None);
                    //mGraphPane.AddCurve(temperatues.Key + "_Exp", expAveragePoints, colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black, SymbolType.None);

                }
            }
            else
            {
                foreach (var temperatues in mTemperatureCount)
                {
                    PointPairList rawPoints = new PointPairList();

                    foreach (var quality in temperatues.Value)
                    {
                        rawPoints.Add(new XDate(quality.Key.ToOADate()), (double)quality.Value);
                    }

                    mGraphPane.AddCurve(temperatues.Key, rawPoints, colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black, SymbolType.None);
                }
            }
            zedGraph.AxisChange();
            zedGraph.Refresh();
        }

        private void TemperatureViewer_FormClosing(object sender, FormClosingEventArgs e)
        {
            mGraphPane.CurveList.Clear();
        }

        private void btnRefreshGraph_Click(object sender, EventArgs e)
        {
            refreshData();
        }
    }
}
