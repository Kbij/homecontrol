/*
 * AdminMapViewer.cs
 *
 * F11's admin live-view: same GMap.NET map setup/rendering as LocationViewer (F12) - same
 * GMapOverlay/GMapPoint/GMapRoute/marker approach, same circle-drawing math (CreateCircle below
 * reuses LocationViewer's FindPointAtDistanceFrom helper) - but fed over the HCM TCP protocol via
 * AdminTcpClient instead of a direct SQL query, and with a geofence circle. Mirrors the Android
 * app's admin map screen (AdminMapScreen.kt / HaRemoteViewModel.kt).
 *
 * The client picker is still populated via LocationDal.fillSources() (the same plain
 * "SELECT clientName FROM Client" the F12 view already uses) rather than the TCP ClientList
 * message (objectId 42) - this app already talks to that database directly everywhere else, so
 * there is no reason to duplicate that lookup over TCP too. Only the actual position/geofence
 * *data* shown on the map comes over TCP, per the request that started this feature.
 *
 * Not PIN-gated: this connects under a fixed configured client name (App.config's
 * AdminClientName) rather than showing a PIN screen like the Android app - see AdminTcpClient's
 * header comment for why that is safe (capability-gated server-side, not session-gated). Koen
 * needs to set an adminCode for that client name in the Client table once; until then every
 * request comes back denied (see OnAuthDenied below).
 */
using GMap.NET;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Drawing;
using System.Windows.Forms;
using WindowsMonitor.DAL;

namespace WindowsMonitor
{
    public partial class AdminMapViewer : Form
    {
        private LocationDal mDal;
        private AdminTcpClient mTcpClient;
        private GMapOverlay mMarkers;
        private GMapOverlay mRoutes;
        private string mSelectedClient;

        public AdminMapViewer()
        {
            InitializeComponent();
        }

        private void AdminMapViewer_Load(object sender, EventArgs e)
        {
            mDal = new LocationDal();

            mapOverview.MapProvider = GMap.NET.MapProviders.GoogleMapProvider.Instance;
            GMap.NET.GMaps.Instance.Mode = GMap.NET.AccessMode.ServerAndCache;
            mapOverview.ShowCenter = false;
            mapOverview.AutoSizeMode = AutoSizeMode.GrowAndShrink;

            mMarkers = new GMapOverlay("Positions");
            mRoutes = new GMapOverlay("Routes");
            mapOverview.Overlays.Add(mMarkers);
            mapOverview.Overlays.Add(mRoutes);

            cmbHours.Items.Add(new Item("15 min", 15));
            cmbHours.Items.Add(new Item("30 min", 30));
            cmbHours.Items.Add(new Item("1", 60));
            cmbHours.Items.Add(new Item("2", 120));
            cmbHours.Items.Add(new Item("6", 360));
            cmbHours.Items.Add(new Item("12", 720));
            cmbHours.Items.Add(new Item("24", 1440));
            cmbHours.SelectedIndex = 0;

            foreach (string client in mDal.fillSources())
            {
                cmbClient.Items.Add(client);
            }
            if (cmbClient.Items.Count > 0) cmbClient.SelectedIndex = 0;

            string clientName = ConfigurationManager.AppSettings["AdminClientName"];
            if (string.IsNullOrEmpty(clientName)) clientName = "WindowsMonitor";
            string host = ConfigurationManager.AppSettings["AdminServerHost"];
            if (string.IsNullOrEmpty(host)) host = "mysql.lan";
            int port;
            if (!int.TryParse(ConfigurationManager.AppSettings["AdminServerPort"], out port)) port = 5678;

            Text = string.Format("Admin live-view (TCP) - client: {0}, {1}:{2}", clientName, host, port);

            mTcpClient = new AdminTcpClient(clientName);
            mTcpClient.ConnectionStateChanged += OnConnectionStateChanged;
            mTcpClient.LocationHistoryReceived += OnLocationHistoryReceived;
            mTcpClient.AuthDenied += OnAuthDenied;
            mTcpClient.Start(host, port);

            UpdateStatus("Verbinden...");
            RequestCurrentSelection();
        }

        private void cmbClient_SelectedIndexChanged(object sender, EventArgs e)
        {
            RequestCurrentSelection();
        }

        private void cmbHours_SelectedIndexChanged(object sender, EventArgs e)
        {
            RequestCurrentSelection();
        }

        private void RequestCurrentSelection()
        {
            if (mTcpClient == null || cmbClient.SelectedItem == null || cmbHours.SelectedItem == null) return;

            string client = cmbClient.SelectedItem.ToString();
            int minutes = (int)((Item)cmbHours.SelectedItem).Value;
            mSelectedClient = client;

            // Clear immediately rather than waiting for the fresh response - mirrors the
            // 10 sept 2026 Android fix (see project notes): otherwise the previous client's
            // marker/route/geofence would keep showing under the newly selected name until the
            // server responds.
            ClearMap();
            lblLastLocation.Text = "Laatste locatie: laden...";
            lblLastConnection.Text = "";
            lblGeofence.Text = "";

            mTcpClient.RequestLocationHistory(client, minutes);
        }

        private void ClearMap()
        {
            mMarkers.Markers.Clear();
            mMarkers.Polygons.Clear();
            mRoutes.Clear();
            mapOverview.Invalidate();
        }

        private void OnConnectionStateChanged(bool connected)
        {
            if (InvokeRequired)
            {
                BeginInvoke((MethodInvoker)delegate { OnConnectionStateChanged(connected); });
                return;
            }
            UpdateStatus(connected ? "Verbonden met server" : "Niet verbonden met server");
            if (connected) RequestCurrentSelection();
        }

        private void OnAuthDenied()
        {
            if (InvokeRequired)
            {
                BeginInvoke((MethodInvoker)OnAuthDenied);
                return;
            }
            UpdateStatus("Geen adminrechten voor deze client - stel Client.adminCode in de databank in.");
        }

        private void OnLocationHistoryReceived(AdminLocationHistory history)
        {
            if (InvokeRequired)
            {
                BeginInvoke((MethodInvoker)delegate { OnLocationHistoryReceived(history); });
                return;
            }

            // A response for a client we've since switched away from; drop it (same guard as
            // AdminMapScreen.kt checking history.client against the currently selected client).
            if (history.Client != mSelectedClient) return;

            ClearMap();

            // Koen (10 sept 2026): when a geofence is active, don't reason about whether a fix
            // landed within the requested time window at all - essentially nothing new comes
            // in while stationary (GPS/network polling is fully paused, see "locatie-polling
            // pauzeren", 8 sept), and the geofence only ever gets cleared the moment the client
            // actually moves. So its own coordinates ARE the current position, guaranteed,
            // whenever it's active - ahead of whatever history.Points says (which can lag
            // behind a renewal, or simply be empty if the window is shorter than the 30-min
            // renewal interval), not merely a fallback for whenever Points happens to be empty.
            AdminLocationPoint last = history.Points.Count > 0 ? history.Points[history.Points.Count - 1] : null;
            bool geofenceActive = history.Geofence != null;
            PointLatLng? markerPosition = geofenceActive
                ? new PointLatLng(history.Geofence.Latitude, history.Geofence.Longitude)
                : (last != null ? (PointLatLng?)new PointLatLng(last.Latitude, last.Longitude) : null);

            if (history.Points.Count > 0)
            {
                List<PointLatLng> points = new List<PointLatLng>();
                foreach (AdminLocationPoint p in history.Points)
                {
                    PointLatLng latLng = new PointLatLng(p.Latitude, p.Longitude);
                    points.Add(latLng);
                    mMarkers.Markers.Add(new GMapPoint(latLng, 6));
                }
                GMapRoute route = new GMapRoute(points, history.Client);
                route.Stroke = new Pen(Color.Red, 3);
                mRoutes.Routes.Add(route);
            }

            if (geofenceActive)
            {
                // Authoritative for "current location" (see markerPosition above) - always
                // show it here, even if history.Points happens to also contain a
                // (necessarily older-or-equal) recent fix.
                lblLastLocation.Text = "Laatste locatie: geofence (bevestigd " +
                    history.Geofence.UpdatedAt.ToString("dd/MM HH:mm:ss") + ")";
            }
            else if (last != null)
            {
                lblLastLocation.Text = "Laatste locatie: " + last.TimeStamp.ToString("dd/MM HH:mm:ss");
            }
            else
            {
                lblLastLocation.Text = "Laatste locatie: (geen data in dit venster)";
            }

            if (markerPosition.HasValue)
            {
                GMarkerGoogle marker = new GMarkerGoogle(markerPosition.Value, GMarkerGoogleType.red_pushpin);
                marker.ToolTipText = geofenceActive
                    ? string.Format("{0} (geofence)", history.Client)
                    : string.Format("{0} {1}", history.Client, last.TimeStamp.ToShortTimeString());
                marker.ToolTipMode = MarkerTooltipMode.Always;
                mMarkers.Markers.Add(marker);
            }

            lblLastConnection.Text = history.LastConnection.HasValue
                ? "Laatste connectie: " + history.LastConnection.Value.ToString("dd/MM HH:mm:ss")
                : "Laatste connectie: onbekend";

            if (geofenceActive)
            {
                CreateCircle(history.Geofence.Latitude, history.Geofence.Longitude, history.Geofence.RadiusMeters);
                lblGeofence.Text = string.Format("Geofence: {0:F0}m, aangemaakt {1}, vernieuwd {2}",
                    history.Geofence.RadiusMeters,
                    history.Geofence.CreatedAt.ToString("dd/MM HH:mm:ss"),
                    history.Geofence.UpdatedAt.ToString("dd/MM HH:mm:ss"));
            }
            else
            {
                lblGeofence.Text = "Geofence: geen actieve geofence";
            }

            // Koen: with only a geofence and no real trajectory (e.g. a short interval like
            // 15 min, where the once-per-30-min renewal fix never lands inside the window),
            // the map never centered - because ZoomAndCenterRoutes fits the map to the bounds
            // of a route's points, and a route with a single point (see the removed
            // single-point-route workaround this used to rely on here) has a zero-size
            // bounding box that GMap.NET doesn't reliably center/zoom on. Setting Position
            // directly sidesteps that entirely and works the same regardless of how many
            // points there are.
            if (history.Points.Count > 0)
            {
                mapOverview.ZoomAndCenterRoutes("Routes");
            }
            else if (markerPosition.HasValue)
            {
                mapOverview.Position = markerPosition.Value;
                if (mapOverview.Zoom < 14) mapOverview.Zoom = 16;
            }
            mapOverview.Invalidate();
        }

        private void UpdateStatus(string text)
        {
            lblStatus.Text = text;
        }

        /// <summary>
        /// Draws the geofence radius as a filled polygon, exactly like LocationViewer's own
        /// CreateCircle (reusing its FindPointAtDistanceFrom trig helper) so this renders
        /// identically to the accuracy circles F12's CSV-load mode already draws.
        /// </summary>
        private void CreateCircle(double lat, double lon, double radiusMeters)
        {
            PointLatLng center = new PointLatLng(lat, lon);
            const int segments = 360;
            List<PointLatLng> circlePoints = new List<PointLatLng>();
            for (int i = 0; i < segments; i++)
            {
                circlePoints.Add(LocationViewer.FindPointAtDistanceFrom(center, i, radiusMeters / 1000.0));
            }
            GMapPolygon polygon = new GMapPolygon(circlePoints, "geofence");
            mMarkers.Polygons.Add(polygon);
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            if (mTcpClient != null)
            {
                mTcpClient.ConnectionStateChanged -= OnConnectionStateChanged;
                mTcpClient.LocationHistoryReceived -= OnLocationHistoryReceived;
                mTcpClient.AuthDenied -= OnAuthDenied;
                mTcpClient.Stop();
            }
            base.OnFormClosed(e);
        }
    }
}
