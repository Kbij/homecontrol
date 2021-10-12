namespace WindowsMonitor
{
    partial class MonitorForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MonitorForm));
            this.listSummary = new System.Windows.Forms.ListView();
            this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colDist = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colAccuracy = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.collastConnected = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colLastLocation = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colLocationInterval = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colBattery = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.mnInterval = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.mnInterval0 = new System.Windows.Forms.ToolStripMenuItem();
            this.mnInterval5 = new System.Windows.Forms.ToolStripMenuItem();
            this.mnInterval30 = new System.Windows.Forms.ToolStripMenuItem();
            this.tmrUpdate = new System.Windows.Forms.Timer(this.components);
            this.cmbHours = new System.Windows.Forms.ComboBox();
            this.btnKML = new System.Windows.Forms.Button();
            this.btnTemperatureViewer = new System.Windows.Forms.Button();
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.backgroundWorker2 = new System.ComponentModel.BackgroundWorker();
            this.cmbClient = new System.Windows.Forms.ComboBox();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.btnCleanup = new System.Windows.Forms.Button();
            this.mnInterval.SuspendLayout();
            this.SuspendLayout();
            // 
            // listSummary
            // 
            this.listSummary.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listSummary.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colName,
            this.colDist,
            this.colAccuracy,
            this.collastConnected,
            this.colLastLocation,
            this.colLocationInterval,
            this.colBattery});
            this.listSummary.ContextMenuStrip = this.mnInterval;
            this.listSummary.FullRowSelect = true;
            this.listSummary.HideSelection = false;
            this.listSummary.Location = new System.Drawing.Point(12, 15);
            this.listSummary.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.listSummary.Name = "listSummary";
            this.listSummary.Size = new System.Drawing.Size(716, 210);
            this.listSummary.TabIndex = 8;
            this.listSummary.UseCompatibleStateImageBehavior = false;
            this.listSummary.View = System.Windows.Forms.View.Details;
            this.listSummary.DoubleClick += new System.EventHandler(this.listSummary_DoubleClick);
            // 
            // colName
            // 
            this.colName.Text = "Name";
            // 
            // colDist
            // 
            this.colDist.Text = "Distance";
            // 
            // colAccuracy
            // 
            this.colAccuracy.Text = "Accuracy";
            // 
            // collastConnected
            // 
            this.collastConnected.Text = "LastConnected";
            this.collastConnected.Width = 120;
            // 
            // colLastLocation
            // 
            this.colLastLocation.Text = "LastLocation";
            this.colLastLocation.Width = 120;
            // 
            // colLocationInterval
            // 
            this.colLocationInterval.Text = "Interval";
            // 
            // colBattery
            // 
            this.colBattery.Text = "Battery";
            // 
            // mnInterval
            // 
            this.mnInterval.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.mnInterval.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnInterval0,
            this.mnInterval5,
            this.mnInterval30});
            this.mnInterval.Name = "mnInterval";
            this.mnInterval.Size = new System.Drawing.Size(95, 76);
            this.mnInterval.Text = "0";
            // 
            // mnInterval0
            // 
            this.mnInterval0.Name = "mnInterval0";
            this.mnInterval0.Size = new System.Drawing.Size(94, 24);
            this.mnInterval0.Text = "0";
            this.mnInterval0.Click += new System.EventHandler(this.mnInterval0_Click);
            // 
            // mnInterval5
            // 
            this.mnInterval5.Name = "mnInterval5";
            this.mnInterval5.Size = new System.Drawing.Size(94, 24);
            this.mnInterval5.Text = "5";
            this.mnInterval5.Click += new System.EventHandler(this.mnInterval5_Click);
            // 
            // mnInterval30
            // 
            this.mnInterval30.Name = "mnInterval30";
            this.mnInterval30.Size = new System.Drawing.Size(94, 24);
            this.mnInterval30.Text = "30";
            this.mnInterval30.Click += new System.EventHandler(this.mnInterval30_Click);
            // 
            // tmrUpdate
            // 
            this.tmrUpdate.Enabled = true;
            this.tmrUpdate.Tick += new System.EventHandler(this.tmrUpdate_Tick);
            // 
            // cmbHours
            // 
            this.cmbHours.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.cmbHours.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbHours.FormattingEnabled = true;
            this.cmbHours.Location = new System.Drawing.Point(12, 231);
            this.cmbHours.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.cmbHours.Name = "cmbHours";
            this.cmbHours.Size = new System.Drawing.Size(124, 24);
            this.cmbHours.TabIndex = 9;
            this.cmbHours.SelectionChangeCommitted += new System.EventHandler(this.cmbHours_SelectionChangeCommitted);
            // 
            // btnKML
            // 
            this.btnKML.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnKML.Location = new System.Drawing.Point(284, 230);
            this.btnKML.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnKML.Name = "btnKML";
            this.btnKML.Size = new System.Drawing.Size(100, 28);
            this.btnKML.TabIndex = 10;
            this.btnKML.Text = "KML";
            this.btnKML.UseVisualStyleBackColor = true;
            this.btnKML.Click += new System.EventHandler(this.btnKML_Click);
            // 
            // btnTemperatureViewer
            // 
            this.btnTemperatureViewer.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnTemperatureViewer.Location = new System.Drawing.Point(624, 233);
            this.btnTemperatureViewer.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.btnTemperatureViewer.Name = "btnTemperatureViewer";
            this.btnTemperatureViewer.Size = new System.Drawing.Size(105, 28);
            this.btnTemperatureViewer.TabIndex = 11;
            this.btnTemperatureViewer.Text = "Temperatuur";
            this.btnTemperatureViewer.UseVisualStyleBackColor = true;
            this.btnTemperatureViewer.Click += new System.EventHandler(this.button1_Click_1);
            // 
            // cmbClient
            // 
            this.cmbClient.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.cmbClient.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbClient.FormattingEnabled = true;
            this.cmbClient.Location = new System.Drawing.Point(147, 231);
            this.cmbClient.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.cmbClient.Name = "cmbClient";
            this.cmbClient.Size = new System.Drawing.Size(128, 24);
            this.cmbClient.TabIndex = 12;
            // 
            // openFileDialog
            // 
            this.openFileDialog.FileName = "openFileDialog";
            // 
            // btnCleanup
            // 
            this.btnCleanup.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCleanup.Location = new System.Drawing.Point(526, 233);
            this.btnCleanup.Name = "btnCleanup";
            this.btnCleanup.Size = new System.Drawing.Size(91, 28);
            this.btnCleanup.TabIndex = 13;
            this.btnCleanup.Text = "Cleanup";
            this.btnCleanup.UseVisualStyleBackColor = true;
            this.btnCleanup.Click += new System.EventHandler(this.btnCleanup_Click);
            // 
            // MonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(745, 268);
            this.Controls.Add(this.btnCleanup);
            this.Controls.Add(this.cmbClient);
            this.Controls.Add(this.btnTemperatureViewer);
            this.Controls.Add(this.btnKML);
            this.Controls.Add(this.cmbHours);
            this.Controls.Add(this.listSummary);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.Name = "MonitorForm";
            this.Text = "HC Monitor";
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.MonitorForm_KeyDown);
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.MonitorForm_KeyPress);
            this.mnInterval.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.ListView listSummary;
        private System.Windows.Forms.ColumnHeader colName;
        private System.Windows.Forms.ColumnHeader colDist;
        private System.Windows.Forms.ColumnHeader collastConnected;
        private System.Windows.Forms.Timer tmrUpdate;
        private System.Windows.Forms.ComboBox cmbHours;
        private System.Windows.Forms.Button btnKML;
        private System.Windows.Forms.ColumnHeader colLastLocation;
        private System.Windows.Forms.Button btnTemperatureViewer;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.ComponentModel.BackgroundWorker backgroundWorker2;
        private System.Windows.Forms.ComboBox cmbClient;
        private System.Windows.Forms.ColumnHeader colAccuracy;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.ContextMenuStrip mnInterval;
        private System.Windows.Forms.ToolStripMenuItem mnInterval0;
        private System.Windows.Forms.ToolStripMenuItem mnInterval5;
        private System.Windows.Forms.ToolStripMenuItem mnInterval30;
        private System.Windows.Forms.ColumnHeader colLocationInterval;
        private System.Windows.Forms.ColumnHeader colBattery;
        private System.Windows.Forms.Button btnCleanup;
    }
}

