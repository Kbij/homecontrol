using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using ThermoSimulation.Interfaces;
using ThermoSimulation.Thermostat;
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
            mGraphPane.XAxis.Type = AxisType.Date;//AxisType.Linear;
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
                if ((mHeaterOnTime - mHeaterOffTime) < mHeaterOnDelaySeconds)
                { // Heater was on for very short time
                    increaseTemp = true;
                }
                else
                { 
                    increaseTemp = mCurrentTime > (mHeaterOnTime + mHeaterOnDelaySeconds) ? true : false;
                }
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

        private List<TemperaturePoint> readSimulationTemperatures(string fileName)
        {
            List<TemperaturePoint> result = new List<TemperaturePoint>();
            using (var reader = new StreamReader(fileName))
            {
                while (!reader.EndOfStream)
                {
                    var line = reader.ReadLine();
                    var values = line.Split(';');
                    result.Add(new TemperaturePoint { DateTime = Convert.ToDateTime(values[1]), Temperature = Convert.ToDouble(values[2]) });
                }
            }

            return result;
        }

        private void twoPointThermo()
        {
            mGraphPane.CurveList.Clear();
            setupWinter();
            mHeaterOnDelaySeconds = 120;
            mHeaterOffDelaySeconds = 10000;
            mThermo = new PidThermostat(10*60, 60); //Heater period = 30 min, min on time = 10 min
            //mThermo = new TwoPointThermostat(0.5);
            mThermo.registerListener(this);
            mSetTemperature = 20;

            PointPairList simulationPoints = new PointPairList();
            PointPairList heaterState = new PointPairList();
            PointPairList heaterOutput = new PointPairList();

            //Every 30 seconds
            for (long time = 0; time < 16 * 60 * 60; time = time + 30)
            {
                if (time > 240) mThermo.setPointChanged(time, mSetTemperature);
                process(time);
                simulationPoints.Add(time / 60, mRoomTemperature);
                heaterState.Add(time / 60, mHeaterState ? 10 : 0);
                PidThermostat pidthermo = mThermo as PidThermostat;
                heaterOutput.Add(time / 60, pidthermo.HeaterOutput/10);

            }
            mGraphPane.AddCurve("Twopoint", simulationPoints, Color.Black, SymbolType.None);
            mGraphPane.AddCurve("Twopoint", heaterState, Color.Black, SymbolType.None);
            mGraphPane.AddCurve("Twopoint", heaterOutput, Color.Black, SymbolType.None);
            zedGraph.AxisChange();
            zedGraph.Refresh();

            mThermo.unRegisterListener();
        }

        private void runDerivative()
        {
            List<TemperaturePoint> temperatures = readSimulationTemperatures(edtSimulationFile.Text);
            Derivative derivative = new Derivative();

            mGraphPane.CurveList.Clear();

            PointPairList rawSimulationPoints = new PointPairList();
            PointPairList simulationPoints = new PointPairList();
            PointPairList firstDerivativePoints = new PointPairList();
            PointPairList secondDerivativePoints = new PointPairList();

            foreach (var temperature in temperatures)
            {
                double avgTemp = derivative.addTemperature(toUnix(temperature.DateTime), temperature.Temperature);
                rawSimulationPoints.Add(new XDate(temperature.DateTime), temperature.Temperature);
                simulationPoints.Add(new XDate(temperature.DateTime), avgTemp);

                if (derivative.isStable())
                {
                    const int TH = 3;
                    decimal first = derivative.firstDerivative() * 50000;
                    decimal second = derivative.secondDerivative() * 100000000;
                    firstDerivativePoints.Add(new XDate(temperature.DateTime), (double) first);
                    secondDerivativePoints.Add(new XDate(temperature.DateTime), (double) second);
                    //if (first >= TH) firstDerivativePoints.Add(new XDate(temperature.DateTime), 15);
                    //if (first > -TH && first < TH) firstDerivativePoints.Add(new XDate(temperature.DateTime), 0);
                    //if (first <= -TH) firstDerivativePoints.Add(new XDate(temperature.DateTime), -15);

                    //if (second >= TH) secondDerivativePoints.Add(new XDate(temperature.DateTime), 10);
                    //if (second > -TH && second < TH) secondDerivativePoints.Add(new XDate(temperature.DateTime), 0);
                    //if (second <= -TH) secondDerivativePoints.Add(new XDate(temperature.DateTime), -10);

                }
            }

            
            mGraphPane.AddCurve("Avg Temperature", rawSimulationPoints, Color.Green, SymbolType.None);
            mGraphPane.AddCurve("Avg Temperature", simulationPoints, Color.Blue, SymbolType.None);
            mGraphPane.AddCurve("First", firstDerivativePoints, Color.Orange, SymbolType.None);
            mGraphPane.AddCurve("Second", secondDerivativePoints, Color.Red, SymbolType.None);
            zedGraph.AxisChange();
            zedGraph.Refresh();
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            runDerivative();
        }
    }
    public class TemperaturePoint
    {
        public DateTime DateTime { get; set; }
        public double Temperature { get; set; }
    }
}
