namespace WindowsMonitor
{
    partial class LocationViewer
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LocationViewer));
            this.mapOverview = new GMap.NET.WindowsForms.GMapControl();
            this.tmrRefresh = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
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
            this.mapOverview.TabIndex = 0;
            this.mapOverview.Zoom = 9D;
            // 
            // tmrRefresh
            // 
            this.tmrRefresh.Tick += new System.EventHandler(this.tmrRefresh_Tick);
            // 
            // LocationViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(823, 503);
            this.Controls.Add(this.mapOverview);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "LocationViewer";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
            this.Text = "LocationViewer";
            this.Load += new System.EventHandler(this.LocationViewer_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private GMap.NET.WindowsForms.GMapControl mapOverview;
        private System.Windows.Forms.Timer tmrRefresh;
    }
}