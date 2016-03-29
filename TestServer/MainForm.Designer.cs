namespace Server
{
    partial class MainForm
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
            this.txtLog = new System.Windows.Forms.TextBox();
            this.btnStart = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.lblLastConnection = new System.Windows.Forms.Label();
            this.listSummary = new System.Windows.Forms.ListView();
            this.colName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colDist = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.collast = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.btnClearKML = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtLog
            // 
            this.txtLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtLog.Location = new System.Drawing.Point(12, 42);
            this.txtLog.Multiline = true;
            this.txtLog.Name = "txtLog";
            this.txtLog.ReadOnly = true;
            this.txtLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtLog.Size = new System.Drawing.Size(529, 381);
            this.txtLog.TabIndex = 0;
            // 
            // btnStart
            // 
            this.btnStart.Location = new System.Drawing.Point(13, 13);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(75, 23);
            this.btnStart.TabIndex = 1;
            this.btnStart.Text = "Start Listener";
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(94, 13);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 3;
            this.button1.Text = "Clear";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(256, 12);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 4;
            this.button2.Text = "KML";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(350, 17);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(74, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Last Location:";
            // 
            // lblLastConnection
            // 
            this.lblLastConnection.AutoSize = true;
            this.lblLastConnection.Location = new System.Drawing.Point(430, 17);
            this.lblLastConnection.Name = "lblLastConnection";
            this.lblLastConnection.Size = new System.Drawing.Size(53, 13);
            this.lblLastConnection.TabIndex = 6;
            this.lblLastConnection.Text = "Unknown";
            // 
            // listSummary
            // 
            this.listSummary.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.listSummary.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colName,
            this.colDist,
            this.collast});
            this.listSummary.FullRowSelect = true;
            this.listSummary.Location = new System.Drawing.Point(559, 42);
            this.listSummary.Name = "listSummary";
            this.listSummary.Size = new System.Drawing.Size(247, 97);
            this.listSummary.TabIndex = 7;
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
            // btnClearKML
            // 
            this.btnClearKML.Location = new System.Drawing.Point(175, 13);
            this.btnClearKML.Name = "btnClearKML";
            this.btnClearKML.Size = new System.Drawing.Size(75, 23);
            this.btnClearKML.TabIndex = 8;
            this.btnClearKML.Text = "Clear KML";
            this.btnClearKML.UseVisualStyleBackColor = true;
            this.btnClearKML.Click += new System.EventHandler(this.btnClearKML_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(815, 435);
            this.Controls.Add(this.btnClearKML);
            this.Controls.Add(this.listSummary);
            this.Controls.Add(this.lblLastConnection);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.btnStart);
            this.Controls.Add(this.txtLog);
            this.Name = "MainForm";
            this.Text = "Server";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtLog;
        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label lblLastConnection;
        private System.Windows.Forms.ListView listSummary;
        private System.Windows.Forms.ColumnHeader colName;
        private System.Windows.Forms.ColumnHeader colDist;
        private System.Windows.Forms.ColumnHeader collast;
        private System.Windows.Forms.Button btnClearKML;
    }
}

