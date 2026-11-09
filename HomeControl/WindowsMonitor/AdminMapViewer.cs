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

        // Always (re)center/zoom on the first response after the user picks a client or window.
        // 31 okt 2026: on the silent tmrRefresh polls too, as long as "Volgen" (chkFollow) is on -
        // since the 29 okt periodic refresh replaced the per-selection request as the only source
        // of new positions, the map otherwise never followed a moving target anymore. To still
        // allow looking around, any user pan/wheel-zoom on the map turns "Volgen" off; tick it
        // again (or pick a client/window) to resume following.
        private bool mCenterOnNextUpdate = true;
        private PointLatLng? mLastMarkerForCenter;
        private List<AdminLocationPoint> mLastPointsForCenter;

        // ---- Movement alarm (29 okt 2026) ----
        // Armed via chkAlarmArmed. On the first update after arming (or after an acknowledge, or
        // after switching client) the current position and geofence state become the baseline.
        // The alarm goes off when, relative to that baseline:
        //   * a geofence that was active is gone or stale (left = movement), or
        //   * the current position is more than nudAlarmDistance meters from the baseline.
        // It then keeps playing a triple beep every tmrAlarmRepeat interval until acknowledged.
        // Acknowledge silences it and re-baselines on the current position; the alarm stays
        // armed, so another X meters of movement alarms again. Uncheck "Alarm actief" to stop
        // watching altogether.
        private bool mAlarmBaselineValid;
        private PointLatLng mAlarmBaselinePosition;
        private bool mAlarmBaselineGeofence;
        private bool mAlarmRinging;
        private PointLatLng? mLastPosition;
        private bool mLastGeofenceActive;
        private string mBaseTitle;

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

            // Only real user interaction stops following; ZoomAndCenterRoutes/Position/Zoom set
            // from code raise neither of these.
            mapOverview.OnMapDrag += StopFollowingByUser;
            mapOverview.MouseWheel += (s, ev) => StopFollowingByUser();

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
            mBaseTitle = Text;

            int alarmDistance;
            if (int.TryParse(ConfigurationManager.AppSettings["AlarmDistanceMeters"], out alarmDistance))
            {
                nudAlarmDistance.Value = Math.Max(nudAlarmDistance.Minimum, Math.Min(nudAlarmDistance.Maximum, alarmDistance));
            }
            int refreshSeconds;
            if (!int.TryParse(ConfigurationManager.AppSettings["AdminRefreshSeconds"], out refreshSeconds) || refreshSeconds < 5)
            {
                refreshSeconds = 30;
            }
            nudAlarmVolume.Value = Math.Max(nudAlarmVolume.Minimum, Math.Min(nudAlarmVolume.Maximum, AlarmBeeper.VolumePercent));
            tmrRefresh.Interval = refreshSeconds * 1000;
            tmrRefresh.Start();
            UpdateAlarmUi();

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
            if (client != mSelectedClient)
            {
                // Another object: its position has nothing to do with the old baseline.
                ResetAlarmBaseline();
                mLastPosition = null;
                mLastGeofenceActive = false;
            }
            mSelectedClient = client;
            mCenterOnNextUpdate = true;
            mLastPointsForCenter = null; // don't let chkFollow_CheckedChanged jump to the old client's data
            mLastMarkerForCenter = null;
            chkFollow.Checked = true; // picking a client/window = "show me where it is now"

            // Clear immediately rather than waiting for the fresh response - mirrors the
            // 10 sept 2026 Android fix (see project notes): otherwise the previous client's
            // marker/route/geofence would keep showing under the newly selected name until the
            // server responds.
            ClearMap();
            lblLastLocation.Text = "Laatste locatie: laden...";
            lblLastConnection.Text = "";
            lblGeofence.Text = "";
            lblSpeed.Text = "";

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
            // 27 okt 2026: ...except when history.Points already holds a fix that is NEWER than
            // the geofence's last renewal. While stationary nothing new is logged, so a fix after
            // UpdatedAt can only mean the client has moved on and the server simply hasn't
            // cleared the geofence (yet). In that case the trajectory wins: the marker goes to
            // the last fix and the stale circle is no longer drawn.
            bool geofenceStale = history.Geofence != null && last != null
                && last.TimeStamp > history.Geofence.UpdatedAt;
            bool geofenceActive = history.Geofence != null && !geofenceStale;
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

            lblSpeed.Text = FormatSpeed(history.Points, geofenceActive);

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
            else if (geofenceStale)
            {
                lblGeofence.Text = string.Format("Geofence: verlaten (nieuwere positie dan vernieuwing {0}, nog niet gewist door server)",
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
            // 28 okt 2026: same zero-size-bounds problem with exactly one point (or several
            // points on the same spot): ZoomAndCenterRoutes then zooms out to country level.
            // Only use it when the points actually span some distance; otherwise fall through
            // to the direct Position/Zoom path below.
            mLastPointsForCenter = history.Points;
            mLastMarkerForCenter = markerPosition;
            if (mCenterOnNextUpdate || chkFollow.Checked)
            {
                if (CenterMap(history.Points, markerPosition)) mCenterOnNextUpdate = false;
            }
            mapOverview.Invalidate();

            mLastPosition = markerPosition;
            mLastGeofenceActive = geofenceActive;
            EvaluateAlarm();
        }

        /// <summary>
        /// Fits the map on the trajectory, or on the single marker position when there is no
        /// real spread (see the 28 okt note above). Returns false when there is nothing to show.
        /// </summary>
        private bool CenterMap(List<AdminLocationPoint> points, PointLatLng? markerPosition)
        {
            if (HasSpread(points))
            {
                mapOverview.ZoomAndCenterRoutes("Routes");
                return true;
            }
            if (markerPosition.HasValue)
            {
                mapOverview.Position = markerPosition.Value;
                if (mapOverview.Zoom < 14) mapOverview.Zoom = 16;
                return true;
            }
            return false;
        }

        /// <summary>
        /// 31 okt 2026: average speed over the selected window = path length along all fixes /
        /// time between the first and the last fix, plus the speed over the last segment
        /// (last two fixes) as a "current" indication. GPS jitter while standing still adds a
        /// little path length, so at very low speeds the average reads slightly high.
        /// </summary>
        private static string FormatSpeed(List<AdminLocationPoint> points, bool geofenceActive)
        {
            if (points == null || points.Count < 2)
            {
                return geofenceActive ? "Snelheid: stilstaand (geofence)" : "Snelheid: onvoldoende punten";
            }

            double totalMeters = 0;
            for (int i = 1; i < points.Count; i++)
            {
                totalMeters += DistanceMeters(
                    new PointLatLng(points[i - 1].Latitude, points[i - 1].Longitude),
                    new PointLatLng(points[i].Latitude, points[i].Longitude));
            }
            double totalSeconds = (points[points.Count - 1].TimeStamp - points[0].TimeStamp).TotalSeconds;
            if (totalSeconds <= 0) return "Snelheid: onbekend (zelfde tijdstempel)";

            double avgKmh = totalMeters / totalSeconds * 3.6;
            string text = string.Format("Snelheid: gem. {0:F0} km/u ({1:F1} km in {2:F0} min)",
                avgKmh, totalMeters / 1000.0, totalSeconds / 60.0);

            AdminLocationPoint a = points[points.Count - 2];
            AdminLocationPoint b = points[points.Count - 1];
            double lastSeconds = (b.TimeStamp - a.TimeStamp).TotalSeconds;
            if (lastSeconds > 0)
            {
                double lastKmh = DistanceMeters(new PointLatLng(a.Latitude, a.Longitude),
                                                new PointLatLng(b.Latitude, b.Longitude)) / lastSeconds * 3.6;
                text += string.Format(", laatst {0:F0} km/u", lastKmh);
            }
            if (geofenceActive) text += " - nu stilstaand (geofence)";
            return text;
        }

        private void StopFollowingByUser()
        {
            if (chkFollow.Checked) chkFollow.Checked = false;
            mCenterOnNextUpdate = false;
        }

        private void chkFollow_CheckedChanged(object sender, EventArgs e)
        {
            // Re-ticked by the user: jump back to the target right away instead of waiting for
            // the next poll.
            if (chkFollow.Checked && mLastPointsForCenter != null)
            {
                CenterMap(mLastPointsForCenter, mLastMarkerForCenter);
                mapOverview.Invalidate();
            }
        }

        /// <summary>
        /// True when the points cover a non-trivial area (more than ~0.0002 deg, about 20m),
        /// i.e. when ZoomAndCenterRoutes has a real bounding box to fit.
        /// </summary>
        private static bool HasSpread(List<AdminLocationPoint> points)
        {
            if (points == null || points.Count < 2) return false;
            double minLat = double.MaxValue, maxLat = double.MinValue;
            double minLon = double.MaxValue, maxLon = double.MinValue;
            foreach (AdminLocationPoint p in points)
            {
                if (p.Latitude < minLat) minLat = p.Latitude;
                if (p.Latitude > maxLat) maxLat = p.Latitude;
                if (p.Longitude < minLon) minLon = p.Longitude;
                if (p.Longitude > maxLon) maxLon = p.Longitude;
            }
            const double minSpan = 0.0002;
            return (maxLat - minLat) > minSpan || (maxLon - minLon) > minSpan;
        }

        // ---------------------------------------------------------------- refresh / alarm

        private void tmrRefresh_Tick(object sender, EventArgs e)
        {
            // Silent poll: no ClearMap; the response handler simply redraws, and re-centers
            // only while "Volgen" is on. Without this the view (and therefore the alarm)
            // only ever updated on a combo change or a reconnect.
            if (mTcpClient == null || string.IsNullOrEmpty(mSelectedClient) || cmbHours.SelectedItem == null) return;
            mTcpClient.RequestLocationHistory(mSelectedClient, (int)((Item)cmbHours.SelectedItem).Value);
        }

        private void chkAlarmArmed_CheckedChanged(object sender, EventArgs e)
        {
            ResetAlarmBaseline();
            if (chkAlarmArmed.Checked)
            {
                EvaluateAlarm(); // takes the baseline right away if a position is already known
            }
            else
            {
                StopRinging();
            }
            UpdateAlarmUi();
        }

        private void btnAlarmAck_Click(object sender, EventArgs e)
        {
            StopRinging();
            ResetAlarmBaseline();
            EvaluateAlarm(); // re-baseline on the current position
            UpdateAlarmUi();
        }

        private void nudAlarmVolume_ValueChanged(object sender, EventArgs e)
        {
            // Rebuilds the WAV at the new level; use "Test alarm" to hear it. Also written back
            // to the exe's config (AlarmVolumePercent) so it survives a restart.
            AlarmBeeper.VolumePercent = (int)nudAlarmVolume.Value;
        }

        private void btnAlarmTest_Click(object sender, EventArgs e)
        {
            AlarmBeeper.PlayTripleBeep();
        }

        private void tmrAlarmRepeat_Tick(object sender, EventArgs e)
        {
            if (mAlarmRinging) AlarmBeeper.PlayTripleBeep();
        }

        private void ResetAlarmBaseline()
        {
            mAlarmBaselineValid = false;
        }

        private void EvaluateAlarm()
        {
            if (!chkAlarmArmed.Checked || !mLastPosition.HasValue)
            {
                UpdateAlarmUi();
                return;
            }

            if (!mAlarmBaselineValid)
            {
                mAlarmBaselinePosition = mLastPosition.Value;
                mAlarmBaselineGeofence = mLastGeofenceActive;
                mAlarmBaselineValid = true;
                UpdateAlarmUi();
                return;
            }

            if (mAlarmRinging)
            {
                UpdateAlarmUi();
                return; // already going off; wait for acknowledge
            }

            double distance = DistanceMeters(mAlarmBaselinePosition, mLastPosition.Value);
            if (mAlarmBaselineGeofence && !mLastGeofenceActive)
            {
                StartRinging(string.Format("ALARM: geofence verlaten ({0:F0} m verplaatst)", distance));
            }
            else if (distance > (double)nudAlarmDistance.Value)
            {
                StartRinging(string.Format("ALARM: {0:F0} m verplaatst", distance));
            }
            else
            {
                UpdateAlarmUi();
            }
        }

        private void StartRinging(string reason)
        {
            mAlarmRinging = true;
            lblAlarmState.Text = reason + " (" + DateTime.Now.ToString("HH:mm:ss") + ")";
            AlarmBeeper.PlayTripleBeep();
            tmrAlarmRepeat.Start();
            UpdateAlarmUi();
        }

        private void StopRinging()
        {
            mAlarmRinging = false;
            tmrAlarmRepeat.Stop();
        }

        private void UpdateAlarmUi()
        {
            btnAlarmAck.Enabled = mAlarmRinging;
            btnAlarmAck.BackColor = mAlarmRinging ? Color.Red : SystemColors.Control;
            btnAlarmAck.ForeColor = mAlarmRinging ? Color.White : SystemColors.ControlText;
            lblAlarmState.ForeColor = mAlarmRinging ? Color.Red : SystemColors.ControlText;
            if (mBaseTitle != null) Text = mAlarmRinging ? "*** ALARM *** " + mBaseTitle : mBaseTitle;

            if (mAlarmRinging) return; // keep the reason text visible until acknowledged
            if (!chkAlarmArmed.Checked)
            {
                lblAlarmState.Text = "";
            }
            else if (!mAlarmBaselineValid)
            {
                lblAlarmState.Text = "wacht op positie...";
            }
            else
            {
                lblAlarmState.Text = mAlarmBaselineGeofence
                    ? "bewaakt (geofence + afstand)"
                    : "bewaakt (afstand)";
            }
        }

        /// <summary>Haversine great-circle distance in meters.</summary>
        private static double DistanceMeters(PointLatLng a, PointLatLng b)
        {
            const double R = 6371000.0;
            double dLat = (b.Lat - a.Lat) * Math.PI / 180.0;
            double dLon = (b.Lng - a.Lng) * Math.PI / 180.0;
            double lat1 = a.Lat * Math.PI / 180.0;
            double lat2 = b.Lat * Math.PI / 180.0;
            double h = Math.Sin(dLat / 2) * Math.Sin(dLat / 2)
                     + Math.Cos(lat1) * Math.Cos(lat2) * Math.Sin(dLon / 2) * Math.Sin(dLon / 2);
            return 2 * R * Math.Asin(Math.Min(1.0, Math.Sqrt(h)));
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
            tmrRefresh.Stop();
            StopRinging();
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
    /// <summary>
    /// Triple beep for the AdminMapViewer movement alarm. Builds a small 16-bit mono PCM WAV in
    /// memory (3 x 180 ms at 1000 Hz with 120 ms gaps) once and plays it asynchronously through
    /// SoundPlayer, so it comes out of the normal speakers at the system volume - unlike
    /// Console.Beep, which on some machines / over RDP is silent.
    /// </summary>
    internal static class AlarmBeeper
    {
        private static System.Media.SoundPlayer sPlayer;
        private static readonly object sLock = new object();

        private static int sVolumePercent = -1;

        /// <summary>
        /// Beep loudness in percent of full scale, set from the "Vol %" box in AdminMapViewer.
        /// Initial value from App.config AlarmVolumePercent (default 15). Applied when the WAV is
        /// built, so it stacks with the Windows/app volume mixer. Setting it rebuilds the WAV on
        /// the next play and saves the value to the exe's .config (best effort).
        /// </summary>
        public static int VolumePercent
        {
            get
            {
                lock (sLock)
                {
                    if (sVolumePercent < 0)
                    {
                        int v;
                        if (!int.TryParse(System.Configuration.ConfigurationManager.AppSettings["AlarmVolumePercent"], out v)) v = 15;
                        sVolumePercent = Math.Max(1, Math.Min(100, v));
                    }
                    return sVolumePercent;
                }
            }
            set
            {
                int v = Math.Max(1, Math.Min(100, value));
                lock (sLock)
                {
                    if (v == sVolumePercent) return;
                    sVolumePercent = v;
                    if (sPlayer != null)
                    {
                        sPlayer.Stop();
                        sPlayer.Dispose();
                        sPlayer = null;
                    }
                }
                SaveVolume(v);
            }
        }

        private static void SaveVolume(int v)
        {
            try
            {
                System.Configuration.Configuration config =
                    System.Configuration.ConfigurationManager.OpenExeConfiguration(System.Configuration.ConfigurationUserLevel.None);
                System.Configuration.KeyValueConfigurationCollection settings = config.AppSettings.Settings;
                if (settings["AlarmVolumePercent"] == null) settings.Add("AlarmVolumePercent", v.ToString());
                else settings["AlarmVolumePercent"].Value = v.ToString();
                config.Save(System.Configuration.ConfigurationSaveMode.Modified);
                System.Configuration.ConfigurationManager.RefreshSection("appSettings");
            }
            catch (Exception)
            {
                // e.g. exe folder not writable - the new level still applies for this session.
            }
        }

        public static void PlayTripleBeep()
        {
            try
            {
                lock (sLock)
                {
                    if (sPlayer == null)
                    {
                        sPlayer = new System.Media.SoundPlayer(new System.IO.MemoryStream(BuildWav()));
                        sPlayer.Load();
                    }
                    sPlayer.Stream.Position = 0;
                    sPlayer.Play();
                }
            }
            catch (Exception)
            {
                // No audio device or similar: fall back to the system sound.
                System.Media.SystemSounds.Exclamation.Play();
            }
        }

        private static byte[] BuildWav()
        {
            const int sampleRate = 22050;
            const double frequency = 1000.0;
            const int beepMs = 180, gapMs = 120, beeps = 3;
            double amplitude = 32767.0 * VolumePercent / 100.0;
            const int fadeSamples = 200; // short ramp in/out to avoid clicks

            int beepSamples = sampleRate * beepMs / 1000;
            int gapSamples = sampleRate * gapMs / 1000;
            int totalSamples = beeps * beepSamples + (beeps - 1) * gapSamples;
            int dataBytes = totalSamples * 2;

            using (System.IO.MemoryStream ms = new System.IO.MemoryStream(44 + dataBytes))
            using (System.IO.BinaryWriter w = new System.IO.BinaryWriter(ms))
            {
                w.Write(System.Text.Encoding.ASCII.GetBytes("RIFF"));
                w.Write(36 + dataBytes);
                w.Write(System.Text.Encoding.ASCII.GetBytes("WAVE"));
                w.Write(System.Text.Encoding.ASCII.GetBytes("fmt "));
                w.Write(16);                 // fmt chunk size
                w.Write((short)1);           // PCM
                w.Write((short)1);           // mono
                w.Write(sampleRate);
                w.Write(sampleRate * 2);     // byte rate
                w.Write((short)2);           // block align
                w.Write((short)16);          // bits per sample
                w.Write(System.Text.Encoding.ASCII.GetBytes("data"));
                w.Write(dataBytes);

                for (int b = 0; b < beeps; b++)
                {
                    for (int i = 0; i < beepSamples; i++)
                    {
                        double env = 1.0;
                        if (i < fadeSamples) env = (double)i / fadeSamples;
                        else if (i > beepSamples - fadeSamples) env = (double)(beepSamples - i) / fadeSamples;
                        w.Write((short)(amplitude * env * Math.Sin(2 * Math.PI * frequency * i / sampleRate)));
                    }
                    if (b < beeps - 1)
                    {
                        for (int i = 0; i < gapSamples; i++) w.Write((short)0);
                    }
                }
                w.Flush();
                return ms.ToArray();
            }
        }
    }
}
