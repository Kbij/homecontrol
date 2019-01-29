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
using ZedGraph;

namespace ThermoSimulation
{
    public partial class MainForm : Form, ThermostatListenerIf
    {
        GraphPane mGraphPane;
        ThermostatIf mThermo;
        double mRoomTemperature;
        double mSetTemperature;
        double mTempChangePerSecond;

        bool mHeaterState;
        long mCurrentTime;
        long mPreviousTime;
        long mHeaterOnTime;
        long mHeaterOffTime;
        long mHeaterOnDelaySeconds;
        long mHeaterOffDelaySeconds;
        public MainForm() 
        {
            InitializeComponent();
            mGraphPane = zedGraph.GraphPane;
            mGraphPane.Title.Text = "Thermo Simulation";
            mGraphPane.XAxis.Title.Text = "Tijd (min)";
            mGraphPane.YAxis.Title.Text = "Temperatuur";
            mGraphPane.XAxis.Type = AxisType.Linear;
            mGraphPane.Legend.IsVisible = true;

        }
        public void heaterOn()
        {
            if (!mHeaterState)
            {
                Debug.WriteLine(string.Format("Heater on, time: {0}", mCurrentTime));
                mHeaterOnTime = mCurrentTime;
                mHeaterState = true;
            }
        }

        public void heaterOff()
        {
            if (mHeaterState)
            {
                Debug.WriteLine(string.Format("Heater off, time: {0}", mCurrentTime));
                mHeaterOffTime = mCurrentTime;
                mHeaterState = false;
            }
        }
        private void process(long time)
        {
            mCurrentTime = time;
            bool increaseTemp = false;
            if (mHeaterState)
            { //Heater is on
                increaseTemp = mCurrentTime > (mHeaterOnTime + mHeaterOnDelaySeconds) ? true : false;
            }
            else
            {
                increaseTemp = mCurrentTime > (mHeaterOffTime + mHeaterOffDelaySeconds) ? false : true;
            }
            // increase or decrease step * seconds passed
            double temperatureSep = mTempChangePerSecond * (mCurrentTime - mPreviousTime);
            mRoomTemperature += increaseTemp ? temperatureSep : -temperatureSep;
            Debug.WriteLine(string.Format("Time: {0}, Increase: {1},  Step: {2}", mCurrentTime, increaseTemp, temperatureSep));

            mPreviousTime = mCurrentTime;
            Debug.WriteLine(string.Format("RoomTemmp: {0}", mRoomTemperature));
            if (mThermo != null) mThermo.temperatureChanged(time, mRoomTemperature);
        }
        private DateTime fromUnix(long unixTime)
        {
            var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return epoch.AddSeconds(unixTime);
        }
        private long toUnix(DateTime date)
        {
            var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return Convert.ToInt64((date - epoch).TotalSeconds);
        }
        private void setupWinter()
        {
            DateTime riseStart = Convert.ToDateTime("18:25");
            DateTime riseEnd = Convert.ToDateTime("21:42");
            DateTime heaterOn = Convert.ToDateTime("18:16");
            DateTime heaterOff = Convert.ToDateTime("21:07");
            double riseStartTemp = 16.21;
            double riseEndTemp = 21.6;

            double riseTimeDiffSeconds = (riseEnd - riseStart).TotalSeconds;
            double riseTempDiff = riseEndTemp - riseStartTemp;
            mTempChangePerSecond = riseTempDiff / riseTimeDiffSeconds;

            mRoomTemperature = 16;
            mSetTemperature = 16;
		    mHeaterState = false;
            mHeaterOnTime = -10000; // Very long time ago
            mHeaterOffTime = -10000; // Very long time ago

		    mHeaterOnDelaySeconds = (int )(riseStart - heaterOn).TotalSeconds;
            mHeaterOffDelaySeconds = (int)(riseEnd - heaterOff).TotalSeconds;
        }

        private void twoPointThermo()
        {
            mGraphPane.CurveList.Clear();
            setupWinter();
            mThermo = new TwoPointThermostat(0.5);
            mThermo.registerListener(this);
            mSetTemperature = 20;

            PointPairList simulationPoints = new PointPairList();
            PointPairList heaterState = new PointPairList();

            //Every 30 seconds
            for (long time = 0; time < 16 * 60 * 60; time = time + 30)
            {
                if (time > 240) mThermo.setPointChanged(time, 20);
                process(time);
                simulationPoints.Add(time / 60, mRoomTemperature);
                heaterState.Add(time / 60, mHeaterState ? 10 : 0);

            }
            mGraphPane.AddCurve("Twopoint", simulationPoints, Color.Black, SymbolType.None);
            mGraphPane.AddCurve("Twopoint", heaterState, Color.Black, SymbolType.None);
            zedGraph.AxisChange();
            zedGraph.Refresh();

            mThermo.unRegisterListener();
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            twoPointThermo();
        }
    }
}
