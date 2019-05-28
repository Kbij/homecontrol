using GMap.NET;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsMonitor.DAL;

namespace WindowsMonitor
{
    public partial class LocationViewer : Form
    {
        Dictionary<string, GpsClient> mLocations;
        LocationDal mDal;
        GMap.NET.WindowsForms.GMapOverlay mMarkers;
        GMapOverlay mRoutes;
        string mLocationFile;
        public LocationViewer()
        {
            InitializeComponent();


        }
        public LocationViewer(string locationFile)
        {
            mLocationFile = locationFile;
            InitializeComponent();
        }

        private void LocationViewer_Load(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(mLocationFile))
            {
                mLocations = new Dictionary<string, GpsClient>();
                mDal = new LocationDal();

                mapOverview.MapProvider = GMap.NET.MapProviders.GoogleMapProvider.Instance;
                GMap.NET.GMaps.Instance.Mode = GMap.NET.AccessMode.ServerAndCache;
                mapOverview.ShowCenter = false;
                mapOverview.AutoSizeMode = AutoSizeMode.GrowAndShrink;

                mMarkers = new GMap.NET.WindowsForms.GMapOverlay("Positions");
                mRoutes = new GMapOverlay("Routes");
                mapOverview.Overlays.Add(mMarkers);
                mapOverview.Overlays.Add(mRoutes);

                List<string> clients = mDal.fillSources();
                foreach (string client in clients)
                {
                    lstSource.Items.Add(client);
                }
                cmbHours.Items.Add(new Item("1", 1));
                cmbHours.Items.Add(new Item("2", 2));
                cmbHours.Items.Add(new Item("6", 6));
                cmbHours.Items.Add(new Item("12", 12));
                cmbHours.Items.Add(new Item("24", 24));
                cmbHours.Items.Add(new Item("48", 48));
                cmbHours.Items.Add(new Item("240", 240));
                cmbHours.Items.Add(new Item("1 Month", 24 * 31));
                cmbHours.Items.Add(new Item("6 Months", 24 * 31 * 6));
                cmbHours.Items.Add(new Item("1y", 24 * 31 * 12));
                cmbHours.Items.Add(new Item("2y", 24 * 31 * 12 * 2));
                cmbHours.Items.Add(new Item("5y", 24 * 31 * 12 * 5));
                cmbHours.SelectedIndex = 0;
                tmrRefresh.Enabled = true;
            }
            else
            {
                loadCsvLocations();
            }
        }

        private void tmrRefresh_Tick(object sender, EventArgs e)
        {
            if (!cbxLive.Checked) return;

            refresh();
            if (tmrRefresh.Interval < 1000)
            {
                tmrRefresh.Interval = 3000;
            }
        }

        private void refresh()
        {
            lock (mLocations)
            {
                Item itm = (Item)cmbHours.SelectedItem;
                mLocations = mDal.fillLastLocation(itm.Value, lstSource.Text);
                Invoke((MethodInvoker)(() => this.updateLocation()));
            }
        }

        private void updateLocation()
        {
            mMarkers.Markers.Clear();
            mRoutes.Clear();

            lock (mLocations)
            {
                foreach (var client in mLocations)
                {
                    if (client.Key == lstSource.Text)
                    {
                        if (client.Value.Locations.Count > 0)
                        {
                            var location = client.Value.Locations.Last();
                            string label = string.Format("{0} {1}", client.Key, location.TimeStamp.ToShortTimeString());
                            GMap.NET.WindowsForms.GMapMarker marker = new GMap.NET.WindowsForms.Markers.GMarkerGoogle(
                                                            new GMap.NET.PointLatLng(location.Latitude, location.Longitude),
                                                            GMap.NET.WindowsForms.Markers.GMarkerGoogleType.blue_pushpin);
                            marker.ToolTipText = label;
                            marker.ToolTipMode = MarkerTooltipMode.Always;
                            mMarkers.Markers.Add(marker);
                        }
                        List<PointLatLng> points = new List<PointLatLng>();

                        foreach (var pos in client.Value.Locations)
                        {
                            points.Add(new PointLatLng(pos.Latitude, pos.Longitude));
                            var point = new GMapPoint(new PointLatLng(pos.Latitude, pos.Longitude), 6);
                            ////GMap.NET.WindowsForms.GMapMarker marker = new GMap.NET.WindowsForms.Markers.GMarkerGoogle(
                            ////                                new GMap.NET.PointLatLng(pos.Latitude, pos.Longitude),
                            ////                                GMap.NET.WindowsForms.Markers.GMark);
                            mMarkers.Markers.Add(point);
                        }
                        GMapRoute route = new GMapRoute(points, client.Key);
                        route.Stroke = new Pen(Color.Red, 3);
                        mRoutes.Routes.Add(route);
                    }
                }
            }

            mapOverview.ZoomAndCenterRoutes("Routes");
            mapOverview.Invalidate();
        }

        private void lstSource_SelectedValueChanged(object sender, EventArgs e)
        {
            refresh();
        }

        private void cmbHours_SelectedIndexChanged(object sender, EventArgs e)
        {
            refresh();
        }
        public static GMap.NET.PointLatLng FindPointAtDistanceFrom(GMap.NET.PointLatLng startPoint, double initialBearingRadians, double distanceKilometres)
        {
            const double radiusEarthKilometres = 6371.01;
            var distRatio = distanceKilometres / radiusEarthKilometres;
            var distRatioSine = Math.Sin(distRatio);
            var distRatioCosine = Math.Cos(distRatio);

            var startLatRad = DegreesToRadians(startPoint.Lat);
            var startLonRad = DegreesToRadians(startPoint.Lng);

            var startLatCos = Math.Cos(startLatRad);
            var startLatSin = Math.Sin(startLatRad);

            var endLatRads = Math.Asin((startLatSin * distRatioCosine) + (startLatCos * distRatioSine * Math.Cos(initialBearingRadians)));

            var endLonRads = startLonRad + Math.Atan2(
                          Math.Sin(initialBearingRadians) * distRatioSine * startLatCos,
                          distRatioCosine - startLatSin * Math.Sin(endLatRads));

            return new GMap.NET.PointLatLng(RadiansToDegrees(endLatRads), RadiansToDegrees(endLonRads));
        }

        public static double DegreesToRadians(double degrees)
        {
            const double degToRadFactor = Math.PI / 180;
            return degrees * degToRadFactor;
        }

        public static double RadiansToDegrees(double radians)
        {
            const double radToDegFactor = 180 / Math.PI;
            return radians * radToDegFactor;
        }
        private void CreateCircle(Double lat, Double lon, double radius)
        {
            PointLatLng point = new PointLatLng(lat, lon);
            int segments = 1000;

            List<PointLatLng> gpollist = new List<PointLatLng>();

            for (int i = 0; i < segments; i++)
                gpollist.Add(FindPointAtDistanceFrom(point, i, radius / 1000));

            GMapPolygon gpol = new GMapPolygon(gpollist, "pol");

            mMarkers.Polygons.Add(gpol);
        }

        private void loadCsvLocations()
        {
            mapOverview.MapProvider = GMap.NET.MapProviders.GoogleMapProvider.Instance;
            GMap.NET.GMaps.Instance.Mode = GMap.NET.AccessMode.ServerAndCache;
            mapOverview.ShowCenter = false;
            mapOverview.AutoSizeMode = AutoSizeMode.GrowAndShrink;

            mMarkers = new GMap.NET.WindowsForms.GMapOverlay("Positions");
            mRoutes = new GMapOverlay("Routes");
            mapOverview.Overlays.Add(mMarkers);
            mapOverview.Overlays.Add(mRoutes);
            using (var reader = new StreamReader(mLocationFile))
            {
                List<PointLatLng> points = new List<PointLatLng>();

                while (!reader.EndOfStream)
                {
                    var line = reader.ReadLine();
                    var values = line.Split(';');
                    double latitude = Convert.ToDouble(values[0]);
                    double longitude = Convert.ToDouble(values[1]);
                    double accuracy = Convert.ToDouble(values[2]);

                    CreateCircle(latitude, longitude, accuracy);

                    points.Add(new PointLatLng(latitude, longitude));
                    var point = new GMapPoint(new PointLatLng(latitude, longitude), 6);
                    mMarkers.Markers.Add(point);

                }

                GMapRoute route = new GMapRoute(points, "");
                route.Stroke = new Pen(Color.Red, 3);
                mRoutes.Routes.Add(route);

            }

            mapOverview.ZoomAndCenterRoutes("Routes");
            mapOverview.Invalidate();

        }

    }
}
