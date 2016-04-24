﻿namespace WindowsMonitor
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
            this.listSummary = new System.Windows.Forms.ListView();
            this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colDist = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.collast = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tmrUpdate = new System.Windows.Forms.Timer(this.components);
            this.cmbHours = new System.Windows.Forms.ComboBox();
            this.btnKML = new System.Windows.Forms.Button();
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
            this.collast});
            this.listSummary.FullRowSelect = true;
            this.listSummary.Location = new System.Drawing.Point(9, 12);
            this.listSummary.Name = "listSummary";
            this.listSummary.Size = new System.Drawing.Size(395, 138);
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
            // collast
            // 
            this.collast.Text = "LastConnected";
            this.collast.Width = 120;
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
            this.cmbHours.Location = new System.Drawing.Point(9, 156);
            this.cmbHours.Name = "cmbHours";
            this.cmbHours.Size = new System.Drawing.Size(94, 21);
            this.cmbHours.TabIndex = 9;
            // 
            // btnKML
            // 
            this.btnKML.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnKML.Location = new System.Drawing.Point(109, 154);
            this.btnKML.Name = "btnKML";
            this.btnKML.Size = new System.Drawing.Size(75, 23);
            this.btnKML.TabIndex = 10;
            this.btnKML.Text = "KML";
            this.btnKML.UseVisualStyleBackColor = true;
            this.btnKML.Click += new System.EventHandler(this.btnKML_Click);
            // 
            // MonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(416, 185);
            this.Controls.Add(this.btnKML);
            this.Controls.Add(this.cmbHours);
            this.Controls.Add(this.listSummary);
            this.Name = "MonitorForm";
            this.Text = "HC Monitor";
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.ListView listSummary;
        private System.Windows.Forms.ColumnHeader colName;
        private System.Windows.Forms.ColumnHeader colDist;
        private System.Windows.Forms.ColumnHeader collast;
        private System.Windows.Forms.Timer tmrUpdate;
        private System.Windows.Forms.ComboBox cmbHours;
        private System.Windows.Forms.Button btnKML;
    }
}
