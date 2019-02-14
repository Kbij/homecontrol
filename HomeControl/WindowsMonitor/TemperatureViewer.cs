using SharedSimulation;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using WindowsMonitor.DAL;
using ZedGraph;

namespace WindowsMonitor
{
    public partial class TemperatureViewer : Form
    {
        GraphPane mGraphPane;
        readonly List<Color> GraphColors = new List<Color> {Color.Red, Color.Green, Color.Lavender, Color.Blue, Color.Brown};

        Dictionary<string, HomeControlData.TemperatureDataTable> mTemperatures;
        Dictionary<string, HomeControlData.RoomHeaterStateDataTable> mRoomHeaterState;
        Dictionary<string, Dictionary<DateTime, int>> mTemperatureCount;

        public TemperatureViewer()
        {
            InitializeComponent();
            mTemperatures = new Dictionary<string, HomeControlData.TemperatureDataTable>();
            mRoomHeaterState = new Dictionary<string, HomeControlData.RoomHeaterStateDataTable>();

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
            mRoomHeaterState.Clear();

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
                    mRoomHeaterState.Remove(remove);
                }

                //Add the new ones to the list
                foreach (ListViewItem item in lstRooms.Items)
                {
                    if (item.Checked)
                    {
                        if (!mTemperatures.ContainsKey(item.ToString()))
                        {
                            HomeControlData.TemperatureDataTable tempTable = new HomeControlData.TemperatureDataTable();
                            HomeControlData.RoomHeaterStateDataTable heaterStateTable = new HomeControlData.RoomHeaterStateDataTable();
                            TemperatureDal dal = new TemperatureDal();
                            DateTime start = dtFrom.Value;
                            DateTime end = dtTo.Value;

                            dal.fillTemperatures(tempTable, start, end, item.Text);
                            dal.fillRoomHeaterState(heaterStateTable, start, end, item.Text);
                            mTemperatures[item.Text] = tempTable;
                            mRoomHeaterState[item.Text] = heaterStateTable;
                            Debug.WriteLine(string.Format("RoomId: {0}, items: {1}", item.Text, tempTable.Count));
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
        private long toUnix(DateTime date)
        {
            var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return Convert.ToInt64((date - epoch).TotalSeconds);
        }
        private double toValue(Trend trend)
        {
            switch(trend)
            {
                case Trend.Rising: return 10;
                case Trend.Stable: return 0;
                case Trend.Falling: return -10;
                default: return 0;
            }
        }
        private double toValue(Acceleration acc)
        {
            switch (acc)
            {
                case Acceleration.Accelerating: return 9;
                case Acceleration.Linear: return 0;
                case Acceleration.Decelerating: return -9;
                default: return 0;
            }
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
                    PointPairList statePoints = new PointPairList();

                    PointPairList firstDerivativePoints = new PointPairList();
                    PointPairList secondDerivativePoints = new PointPairList();

                    PointPairList trendPoints = new PointPairList();
                    PointPairList accelerationPoints = new PointPairList();

                    DateTime prevTime = DateTime.Now;
                    Derivative derivative = new Derivative();

                    foreach (HomeControlData.TemperatureRow temp in temperatues.Value)
                    {
                        rawPoints.Add(new XDate(temp.date.ToOADate()), (double)temp.temperature);
                        double avgTemp = derivative.addTemperature(toUnix(temp.date), (double)temp.temperature);
                        if (derivative.isStable())
                        {
                            decimal first = derivative.firstDerivative() * 50000;
                            decimal second = derivative.secondDerivative() * 100000000;
                            firstDerivativePoints.Add(new XDate(temp.date), (double)first);
                            secondDerivativePoints.Add(new XDate(temp.date), (double)second);
                            trendPoints.Add(new XDate(temp.date), toValue(derivative.temperatureTrend()));
                            accelerationPoints.Add(new XDate(temp.date), toValue(derivative.temperatureAccelaration()));
                        }
                    }

                    //Lookup the corresponding heaterstate list
                    foreach (var states in mRoomHeaterState)
                    {
                        if (states.Key == temperatues.Key)
                        {//Found it
                            bool prevState = false;
                            foreach(var state in states.Value)
                            {
                                if (state.heater != prevState)
                                {//If there is a difference
                                    statePoints.Add(new XDate(state.date.AddSeconds(-1).ToOADate()), !state.heater ? 10 : 0);
                                }
                                statePoints.Add(new XDate(state.date.ToOADate()), state.heater ? 10 : 0);
                                prevState = state.heater;
                            }
                        }
                         
                    }
                    Color graphColor = colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black;
                    mGraphPane.AddCurve(temperatues.Key, rawPoints, graphColor, SymbolType.None);

                    graphColor = colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black;
                    mGraphPane.AddCurve(temperatues.Key + "ST", statePoints, graphColor, SymbolType.None);

                    //graphColor = colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black;
                    //mGraphPane.AddCurve(temperatues.Key + "'", firstDerivativePoints, graphColor, SymbolType.None);

                    //graphColor = colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black;
                    //mGraphPane.AddCurve(temperatues.Key + "''", secondDerivativePoints, graphColor, SymbolType.None);

                    graphColor = colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black;
                    mGraphPane.AddCurve(temperatues.Key + "TR", trendPoints, graphColor, SymbolType.None);

                    graphColor = colorPos < GraphColors.Count ? GraphColors[colorPos++] : Color.Black;
                    mGraphPane.AddCurve(temperatues.Key + "AC", accelerationPoints, graphColor, SymbolType.None);

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

        private void btnSaveRaw_Click(object sender, EventArgs e)
        {
            SaveFileDialog fileDialog = new SaveFileDialog();
            fileDialog.Filter = "csv files (*.csv)|*.csv";
            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                using (StreamWriter outputFile = new StreamWriter(fileDialog.FileName))
                {
                    foreach (var temperatues in mTemperatures)
                    {
                        foreach (HomeControlData.TemperatureRow temp in temperatues.Value)
                        {
                            outputFile.WriteLine(string.Format("{0};{1};{2}", temperatues.Key, temp.date, temp.temperature));
                        }
                    }
                }
            }

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                using (StreamWriter outputFile = new StreamWriter(fileDialog.FileName))
                {
                    foreach (var states in mRoomHeaterState)
                    {
                        foreach (var state in states.Value)
                        {
                            outputFile.WriteLine(string.Format("{0};{1};{2}", states.Key, state.date, state.heater));
                        }
                    }
                }
            }
        }

    }
}
