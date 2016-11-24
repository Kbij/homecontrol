namespace WindowsMonitor
{
    partial class TemperatureViewer
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
            this.zedGraph = new ZedGraph.ZedGraphControl();
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.lstRooms = new System.Windows.Forms.ListView();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.dtFrom = new System.Windows.Forms.DateTimePicker();
            this.dtTo = new System.Windows.Forms.DateTimePicker();
            this.btnRefreshGraph = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // zedGraph
            // 
            this.zedGraph.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.zedGraph.Location = new System.Drawing.Point(12, 128);
            this.zedGraph.Name = "zedGraph";
            this.zedGraph.ScrollGrace = 0D;
            this.zedGraph.ScrollMaxX = 0D;
            this.zedGraph.ScrollMaxY = 0D;
            this.zedGraph.ScrollMaxY2 = 0D;
            this.zedGraph.ScrollMinX = 0D;
            this.zedGraph.ScrollMinY = 0D;
            this.zedGraph.ScrollMinY2 = 0D;
            this.zedGraph.Size = new System.Drawing.Size(810, 318);
            this.zedGraph.TabIndex = 0;
            this.zedGraph.UseExtendedPrintDialog = true;
            // 
            // lstRooms
            // 
            this.lstRooms.CheckBoxes = true;
            this.lstRooms.Location = new System.Drawing.Point(12, 31);
            this.lstRooms.MultiSelect = false;
            this.lstRooms.Name = "lstRooms";
            this.lstRooms.Size = new System.Drawing.Size(197, 78);
            this.lstRooms.TabIndex = 3;
            this.lstRooms.UseCompatibleStateImageBehavior = false;
            this.lstRooms.View = System.Windows.Forms.View.List;
            this.lstRooms.ItemChecked += new System.Windows.Forms.ItemCheckedEventHandler(this.lstRooms_ItemChecked);
            // 
            // btnRefresh
            // 
            this.btnRefresh.Location = new System.Drawing.Point(12, 2);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(75, 23);
            this.btnRefresh.TabIndex = 4;
            this.btnRefresh.Text = "Refresh";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // dtFrom
            // 
            this.dtFrom.CustomFormat = "dd/MM/yyyy HH:mm";
            this.dtFrom.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dtFrom.Location = new System.Drawing.Point(215, 31);
            this.dtFrom.Name = "dtFrom";
            this.dtFrom.Size = new System.Drawing.Size(200, 20);
            this.dtFrom.TabIndex = 5;
            // 
            // dtTo
            // 
            this.dtTo.CustomFormat = "dd/MM/yyyy HH:mm";
            this.dtTo.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dtTo.Location = new System.Drawing.Point(215, 57);
            this.dtTo.Name = "dtTo";
            this.dtTo.Size = new System.Drawing.Size(200, 20);
            this.dtTo.TabIndex = 6;
            // 
            // btnRefreshGraph
            // 
            this.btnRefreshGraph.Location = new System.Drawing.Point(216, 85);
            this.btnRefreshGraph.Name = "btnRefreshGraph";
            this.btnRefreshGraph.Size = new System.Drawing.Size(118, 23);
            this.btnRefreshGraph.TabIndex = 7;
            this.btnRefreshGraph.Text = "Refresh Graph";
            this.btnRefreshGraph.UseVisualStyleBackColor = true;
            this.btnRefreshGraph.Click += new System.EventHandler(this.btnRefreshGraph_Click);
            // 
            // TemperatureViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(834, 471);
            this.Controls.Add(this.btnRefreshGraph);
            this.Controls.Add(this.dtTo);
            this.Controls.Add(this.dtFrom);
            this.Controls.Add(this.btnRefresh);
            this.Controls.Add(this.lstRooms);
            this.Controls.Add(this.zedGraph);
            this.Name = "TemperatureViewer";
            this.Text = "TemperatureViewer";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.TemperatureViewer_FormClosing);
            this.ResumeLayout(false);

        }

        #endregion

        private ZedGraph.ZedGraphControl zedGraph;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.Windows.Forms.ListView lstRooms;
        private System.Windows.Forms.Button btnRefresh;
        private System.Windows.Forms.DateTimePicker dtFrom;
        private System.Windows.Forms.DateTimePicker dtTo;
        private System.Windows.Forms.Button btnRefreshGraph;
    }
}