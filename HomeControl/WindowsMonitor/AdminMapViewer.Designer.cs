namespace WindowsMonitor
{
    partial class AdminMapViewer
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.mapOverview = new GMap.NET.WindowsForms.GMapControl();
            this.cmbClient = new System.Windows.Forms.ComboBox();
            this.cmbHours = new System.Windows.Forms.ComboBox();
            this.lblStatus = new System.Windows.Forms.Label();
            this.lblLastLocation = new System.Windows.Forms.Label();
            this.lblLastConnection = new System.Windows.Forms.Label();
            this.lblGeofence = new System.Windows.Forms.Label();
            this.SuspendLayout();
            //
            // cmbClient
            //
            this.cmbClient.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbClient.FormattingEnabled = true;
            this.cmbClient.Location = new System.Drawing.Point(0, 0);
            this.cmbClient.Name = "cmbClient";
            this.cmbClient.Size = new System.Drawing.Size(174, 21);
            this.cmbClient.TabIndex = 0;
            this.cmbClient.SelectedIndexChanged += new System.EventHandler(this.cmbClient_SelectedIndexChanged);
            //
            // cmbHours
            //
            this.cmbHours.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbHours.FormattingEnabled = true;
            this.cmbHours.Location = new System.Drawing.Point(181, 0);
            this.cmbHours.Name = "cmbHours";
            this.cmbHours.Size = new System.Drawing.Size(90, 21);
            this.cmbHours.TabIndex = 1;
            this.cmbHours.SelectedIndexChanged += new System.EventHandler(this.cmbHours_SelectedIndexChanged);
            //
            // lblStatus
            //
            this.lblStatus.AutoSize = true;
            this.lblStatus.Location = new System.Drawing.Point(278, 4);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(120, 13);
            this.lblStatus.TabIndex = 2;
            this.lblStatus.Text = "Niet verbonden";
            //
            // lblLastLocation
            //
            this.lblLastLocation.AutoSize = true;
            this.lblLastLocation.Location = new System.Drawing.Point(0, 26);
            this.lblLastLocation.Name = "lblLastLocation";
            this.lblLastLocation.Size = new System.Drawing.Size(90, 13);
            this.lblLastLocation.TabIndex = 4;
            this.lblLastLocation.Text = "Laatste locatie:";
            //
            // lblLastConnection
            //
            this.lblLastConnection.AutoSize = true;
            this.lblLastConnection.Location = new System.Drawing.Point(270, 26);
            this.lblLastConnection.Name = "lblLastConnection";
            this.lblLastConnection.Size = new System.Drawing.Size(105, 13);
            this.lblLastConnection.TabIndex = 5;
            this.lblLastConnection.Text = "Laatste connectie:";
            //
            // lblGeofence
            //
            this.lblGeofence.AutoSize = true;
            this.lblGeofence.Location = new System.Drawing.Point(540, 26);
            this.lblGeofence.Name = "lblGeofence";
            this.lblGeofence.Size = new System.Drawing.Size(60, 13);
            this.lblGeofence.TabIndex = 6;
            this.lblGeofence.Text = "Geofence:";
            //
            // mapOverview
            //
            this.mapOverview.Bearing = 0F;
            this.mapOverview.CanDragMap = true;
            this.mapOverview.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mapOverview.EmptyTileColor = System.Drawing.Color.Navy;
            this.mapOverview.GrayScaleMode = false;
            this.mapOverview.HelperLineOption = GMap.NET.WindowsForms.HelperLineOptions.DontShow;
            this.mapOverview.LevelsKeepInMemmory = 5;
            this.mapOverview.Location = new System.Drawing.Point(0, 0);
            this.mapOverview.MarkersEnabled = true;
            this.mapOverview.MaxZoom = 18;
            this.mapOverview.MinZoom = 2;
            this.mapOverview.MouseWheelZoomType = GMap.NET.MouseWheelZoomType.MousePositionAndCenter;
            this.mapOverview.Name = "mapOverview";
            this.mapOverview.NegativeMode = false;
            this.mapOverview.PolygonsEnabled = true;
            this.mapOverview.RetryLoadTile = 0;
            this.mapOverview.RoutesEnabled = true;
            this.mapOverview.ScaleMode = GMap.NET.WindowsForms.ScaleModes.Integer;
            this.mapOverview.SelectedAreaFillColor = System.Drawing.Color.FromArgb(((int)(((byte)(33)))), ((int)(((byte)(65)))), ((int)(((byte)(105)))), ((int)(((byte)(225)))));
            this.mapOverview.ShowTileGridLines = false;
            this.mapOverview.Size = new System.Drawing.Size(823, 503);
            this.mapOverview.TabIndex = 7;
            this.mapOverview.Zoom = 9D;
            //
            // AdminMapViewer
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(823, 503);
            this.Controls.Add(this.lblGeofence);
            this.Controls.Add(this.lblLastConnection);
            this.Controls.Add(this.lblLastLocation);
            this.Controls.Add(this.lblStatus);
            this.Controls.Add(this.cmbHours);
            this.Controls.Add(this.cmbClient);
            this.Controls.Add(this.mapOverview);
            this.Name = "AdminMapViewer";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
            this.Text = "Admin live-view (TCP)";
            this.Load += new System.EventHandler(this.AdminMapViewer_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private GMap.NET.WindowsForms.GMapControl mapOverview;
        private System.Windows.Forms.ComboBox cmbClient;
        private System.Windows.Forms.ComboBox cmbHours;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Label lblLastLocation;
        private System.Windows.Forms.Label lblLastConnection;
        private System.Windows.Forms.Label lblGeofence;
    }
}
