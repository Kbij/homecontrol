using GMap.NET;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsMonitor.DAL;

namespace WindowsMonitor
{
    public partial class LocationViewer : Form
    {
        Dictionary<Tuple<string, DateTime>, List<GpsLocation>> mLocations;
        LocationDal mDal;
        GMap.NET.WindowsForms.GMapOverlay mMarkers;
        GMapOverlay mRoutes;
        private string mClientName;
        public LocationViewer(string clientName)
        {
            InitializeComponent();
            mClientName = clientName;
        }

        private void LocationViewer_Load(object sender, EventArgs e)
        {
            mLocations = new Dictionary<Tuple<string, DateTime>, List<GpsLocation>>();
            mDal = new LocationDal();

            mapOverview.MapProvider = GMap.NET.MapProviders.GoogleMapProvider.Instance;
            GMap.NET.GMaps.Instance.Mode = GMap.NET.AccessMode.ServerAndCache;
            mapOverview.ShowCenter = false;
            mapOverview.AutoSizeMode = AutoSizeMode.GrowAndShrink;

            mMarkers = new GMap.NET.WindowsForms.GMapOverlay("Positions");
            mRoutes = new GMapOverlay("Routes");
            mapOverview.Overlays.Add(mMarkers);
            mapOverview.Overlays.Add(mRoutes);

            tmrRefresh.Enabled = true;
        }

        private void tmrRefresh_Tick(object sender, EventArgs e)
        {
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
                mLocations = mDal.fillLastLocation(1);
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
                    if (client.Key.Item1 == mClientName)
                    {
                        if (client.Value.Count > 0)
                        {
                            var location = client.Value.Last();
                            string label = string.Format("{0} {1}", client.Key.Item1, location.TimeStamp.ToShortTimeString());
                            GMap.NET.WindowsForms.GMapMarker marker = new GMap.NET.WindowsForms.Markers.GMarkerGoogle(
                                                            new GMap.NET.PointLatLng(location.Latitude, location.Longitude),
                                                            GMap.NET.WindowsForms.Markers.GMarkerGoogleType.blue_pushpin);
                            marker.ToolTipText = label;
                            marker.ToolTipMode = MarkerTooltipMode.Always;
                            mMarkers.Markers.Add(marker);
                        }
                        List<PointLatLng> points = new List<PointLatLng>();

                        foreach (var pos in client.Value)
                        {
                            points.Add(new PointLatLng(pos.Latitude, pos.Longitude));
                            var point = new GMapPoint(new PointLatLng(pos.Latitude, pos.Longitude), 6);
                            ////GMap.NET.WindowsForms.GMapMarker marker = new GMap.NET.WindowsForms.Markers.GMarkerGoogle(
                            ////                                new GMap.NET.PointLatLng(pos.Latitude, pos.Longitude),
                            ////                                GMap.NET.WindowsForms.Markers.GMark);
                            mMarkers.Markers.Add(point);
                        }
                        GMapRoute route = new GMapRoute(points, client.Key.Item1);
                        route.Stroke = new Pen(Color.Red, 3);
                        mRoutes.Routes.Add(route);
                    }
                }
            }

            mapOverview.ZoomAndCenterRoutes("Routes");
            mapOverview.Invalidate();
        }
    }
}
