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
            this.chkAlarmArmed = new System.Windows.Forms.CheckBox();
            this.nudAlarmDistance = new System.Windows.Forms.NumericUpDown();
            this.lblAlarmMeters = new System.Windows.Forms.Label();
            this.btnAlarmAck = new System.Windows.Forms.Button();
            this.btnAlarmTest = new System.Windows.Forms.Button();
            this.lblAlarmState = new System.Windows.Forms.Label();
            this.lblAlarmVolume = new System.Windows.Forms.Label();
            this.nudAlarmVolume = new System.Windows.Forms.NumericUpDown();
            this.chkFollow = new System.Windows.Forms.CheckBox();
            this.lblSpeed = new System.Windows.Forms.Label();
            this.tmrRefresh = new System.Windows.Forms.Timer(this.components);
            this.tmrAlarmRepeat = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.nudAlarmDistance)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudAlarmVolume)).BeginInit();
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
            // chkAlarmArmed
            //
            this.chkAlarmArmed.AutoSize = true;
            this.chkAlarmArmed.Location = new System.Drawing.Point(420, 2);
            this.chkAlarmArmed.Name = "chkAlarmArmed";
            this.chkAlarmArmed.Size = new System.Drawing.Size(84, 17);
            this.chkAlarmArmed.TabIndex = 8;
            this.chkAlarmArmed.Text = "Alarm actief";
            this.chkAlarmArmed.UseVisualStyleBackColor = true;
            this.chkAlarmArmed.CheckedChanged += new System.EventHandler(this.chkAlarmArmed_CheckedChanged);
            //
            // nudAlarmDistance
            //
            this.nudAlarmDistance.Increment = new decimal(new int[] { 50, 0, 0, 0 });
            this.nudAlarmDistance.Location = new System.Drawing.Point(508, 1);
            this.nudAlarmDistance.Maximum = new decimal(new int[] { 100000, 0, 0, 0 });
            this.nudAlarmDistance.Minimum = new decimal(new int[] { 10, 0, 0, 0 });
            this.nudAlarmDistance.Name = "nudAlarmDistance";
            this.nudAlarmDistance.Size = new System.Drawing.Size(60, 20);
            this.nudAlarmDistance.TabIndex = 9;
            this.nudAlarmDistance.Value = new decimal(new int[] { 200, 0, 0, 0 });
            //
            // lblAlarmMeters
            //
            this.lblAlarmMeters.AutoSize = true;
            this.lblAlarmMeters.Location = new System.Drawing.Point(570, 4);
            this.lblAlarmMeters.Name = "lblAlarmMeters";
            this.lblAlarmMeters.Size = new System.Drawing.Size(15, 13);
            this.lblAlarmMeters.TabIndex = 10;
            this.lblAlarmMeters.Text = "m";
            //
            // btnAlarmAck
            //
            this.btnAlarmAck.Enabled = false;
            this.btnAlarmAck.Location = new System.Drawing.Point(592, 0);
            this.btnAlarmAck.Name = "btnAlarmAck";
            this.btnAlarmAck.Size = new System.Drawing.Size(80, 22);
            this.btnAlarmAck.TabIndex = 11;
            this.btnAlarmAck.Text = "Acknowledge";
            this.btnAlarmAck.UseVisualStyleBackColor = true;
            this.btnAlarmAck.Click += new System.EventHandler(this.btnAlarmAck_Click);
            //
            // btnAlarmTest
            //
            this.btnAlarmTest.Location = new System.Drawing.Point(676, 0);
            this.btnAlarmTest.Name = "btnAlarmTest";
            this.btnAlarmTest.Size = new System.Drawing.Size(70, 22);
            this.btnAlarmTest.TabIndex = 12;
            this.btnAlarmTest.Text = "Test alarm";
            this.btnAlarmTest.UseVisualStyleBackColor = true;
            this.btnAlarmTest.Click += new System.EventHandler(this.btnAlarmTest_Click);
            //
            // lblAlarmState
            //
            this.lblAlarmState.AutoSize = true;
            this.lblAlarmState.Location = new System.Drawing.Point(900, 4);
            this.lblAlarmState.Name = "lblAlarmState";
            this.lblAlarmState.Size = new System.Drawing.Size(0, 13);
            this.lblAlarmState.TabIndex = 13;
            //
            // lblAlarmVolume
            //
            this.lblAlarmVolume.AutoSize = true;
            this.lblAlarmVolume.Location = new System.Drawing.Point(750, 4);
            this.lblAlarmVolume.Name = "lblAlarmVolume";
            this.lblAlarmVolume.Size = new System.Drawing.Size(31, 13);
            this.lblAlarmVolume.TabIndex = 14;
            this.lblAlarmVolume.Text = "Vol %";
            //
            // nudAlarmVolume
            //
            this.nudAlarmVolume.Increment = new decimal(new int[] { 5, 0, 0, 0 });
            this.nudAlarmVolume.Location = new System.Drawing.Point(785, 1);
            this.nudAlarmVolume.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            this.nudAlarmVolume.Name = "nudAlarmVolume";
            this.nudAlarmVolume.Size = new System.Drawing.Size(48, 20);
            this.nudAlarmVolume.TabIndex = 15;
            this.nudAlarmVolume.Value = new decimal(new int[] { 15, 0, 0, 0 });
            this.nudAlarmVolume.ValueChanged += new System.EventHandler(this.nudAlarmVolume_ValueChanged);
            //
            // chkFollow
            //
            this.chkFollow.AutoSize = true;
            this.chkFollow.Checked = true;
            this.chkFollow.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkFollow.Location = new System.Drawing.Point(840, 2);
            this.chkFollow.Name = "chkFollow";
            this.chkFollow.Size = new System.Drawing.Size(58, 17);
            this.chkFollow.TabIndex = 16;
            this.chkFollow.Text = "Volgen";
            this.chkFollow.UseVisualStyleBackColor = true;
            this.chkFollow.CheckedChanged += new System.EventHandler(this.chkFollow_CheckedChanged);
            //
            // lblSpeed
            //
            this.lblSpeed.AutoSize = true;
            this.lblSpeed.Location = new System.Drawing.Point(0, 44);
            this.lblSpeed.Name = "lblSpeed";
            this.lblSpeed.Size = new System.Drawing.Size(55, 13);
            this.lblSpeed.TabIndex = 17;
            this.lblSpeed.Text = "Snelheid:";
            //
            // tmrRefresh
            //
            this.tmrRefresh.Interval = 30000;
            this.tmrRefresh.Tick += new System.EventHandler(this.tmrRefresh_Tick);
            //
            // tmrAlarmRepeat
            //
            this.tmrAlarmRepeat.Interval = 4000;
            this.tmrAlarmRepeat.Tick += new System.EventHandler(this.tmrAlarmRepeat_Tick);
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
            this.ClientSize = new System.Drawing.Size(1100, 600);
            this.Controls.Add(this.lblSpeed);
            this.Controls.Add(this.chkFollow);
            this.Controls.Add(this.lblAlarmState);
            this.Controls.Add(this.nudAlarmVolume);
            this.Controls.Add(this.lblAlarmVolume);
            this.Controls.Add(this.btnAlarmTest);
            this.Controls.Add(this.btnAlarmAck);
            this.Controls.Add(this.lblAlarmMeters);
            this.Controls.Add(this.nudAlarmDistance);
            this.Controls.Add(this.chkAlarmArmed);
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
            ((System.ComponentModel.ISupportInitialize)(this.nudAlarmDistance)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudAlarmVolume)).EndInit();
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
        private System.Windows.Forms.CheckBox chkAlarmArmed;
        private System.Windows.Forms.NumericUpDown nudAlarmDistance;
        private System.Windows.Forms.Label lblAlarmMeters;
        private System.Windows.Forms.Button btnAlarmAck;
        private System.Windows.Forms.Button btnAlarmTest;
        private System.Windows.Forms.Label lblAlarmState;
        private System.Windows.Forms.Label lblAlarmVolume;
        private System.Windows.Forms.NumericUpDown nudAlarmVolume;
        private System.Windows.Forms.CheckBox chkFollow;
        private System.Windows.Forms.Label lblSpeed;
        private System.Windows.Forms.Timer tmrRefresh;
        private System.Windows.Forms.Timer tmrAlarmRepeat;
    }
}
