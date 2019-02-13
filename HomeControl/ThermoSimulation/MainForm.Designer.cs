namespace ThermoSimulation
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
            this.components = new System.ComponentModel.Container();
            this.zedGraph = new ZedGraph.ZedGraphControl();
            this.btnRun = new System.Windows.Forms.Button();
            this.edtSimulationFile = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // zedGraph
            // 
            this.zedGraph.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.zedGraph.Location = new System.Drawing.Point(12, 3);
            this.zedGraph.Name = "zedGraph";
            this.zedGraph.ScrollGrace = 0D;
            this.zedGraph.ScrollMaxX = 0D;
            this.zedGraph.ScrollMaxY = 0D;
            this.zedGraph.ScrollMaxY2 = 0D;
            this.zedGraph.ScrollMinX = 0D;
            this.zedGraph.ScrollMinY = 0D;
            this.zedGraph.ScrollMinY2 = 0D;
            this.zedGraph.Size = new System.Drawing.Size(915, 441);
            this.zedGraph.TabIndex = 0;
            this.zedGraph.UseExtendedPrintDialog = true;
            // 
            // btnRun
            // 
            this.btnRun.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnRun.Location = new System.Drawing.Point(12, 476);
            this.btnRun.Name = "btnRun";
            this.btnRun.Size = new System.Drawing.Size(141, 23);
            this.btnRun.TabIndex = 1;
            this.btnRun.Text = "Run Simulation";
            this.btnRun.UseVisualStyleBackColor = true;
            this.btnRun.Click += new System.EventHandler(this.btnRun_Click);
            // 
            // edtSimulationFile
            // 
            this.edtSimulationFile.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.edtSimulationFile.Location = new System.Drawing.Point(12, 450);
            this.edtSimulationFile.Name = "edtSimulationFile";
            this.edtSimulationFile.Size = new System.Drawing.Size(335, 20);
            this.edtSimulationFile.TabIndex = 2;
            this.edtSimulationFile.Text = "Living_29-1_02-02.csv";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(939, 511);
            this.Controls.Add(this.edtSimulationFile);
            this.Controls.Add(this.btnRun);
            this.Controls.Add(this.zedGraph);
            this.Name = "MainForm";
            this.Text = "Simulation";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl zedGraph;
        private System.Windows.Forms.Button btnRun;
        private System.Windows.Forms.TextBox edtSimulationFile;
    }
}

