using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

using System.Reflection;

namespace LogHost
{
    /// <summary>
    /// Displays incoming log entries.
    /// Provides Start, Stop and Clear capabilities
    /// </summary>
    public partial class LogForm : Form
    {
        private const int defaultPort = 8001; // Default port to listen on
        private UdpReceiver receiver;

        public LogForm()
        {
            InitializeComponent();            
            this.Text = string.Format("Logging Console V{0}", Assembly.GetExecutingAssembly().GetName().Version.ToString());
            btnStart.Click += btnStart_Click;
            btnStop.Click += btnStop_Click;
            btnClear.Click += btnClear_Click;
            UpdateControlStatus(LogStatus.Stopped);
        }

        delegate void ShowLogDelegate(string data);

        private void ShowLog(string data)
        {
            if (listBox1.InvokeRequired)
            {
                BeginInvoke(new ShowLogDelegate(ShowLog), data);
            }
            else
            {
                listBox1.Items.Add(data);
                int visibleItems = listBox1.ClientSize.Height / listBox1.ItemHeight;
                listBox1.TopIndex = Math.Max(listBox1.Items.Count - visibleItems + 1, 0);
            }
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            base.OnClosing(e);
            if (receiver != null)
            {
                receiver.LogEntryReceived -= receiver_LogEntryReceived;
                receiver = null;
            }
        }

        /// <summary>
        /// Enable and disable controls depending on program status
        /// </summary>
        private void UpdateControlStatus(LogStatus status)
        {
            txtPort.Enabled = (status == LogStatus.Stopped);
            btnStop.Enabled = (status == LogStatus.Running);
            btnStart.Enabled = (status == LogStatus.Stopped);
            btnClear.Enabled = (status == LogStatus.Stopped);
        }

        private void InitialiseReceiver()
        {
            if (receiver != null)
            {
                receiver.LogEntryReceived -= receiver_LogEntryReceived;
                receiver = null;
            }
            int port;
            if (!int.TryParse(txtPort.Text, out port))
                port = defaultPort;
            receiver = new UdpReceiver(port);
            receiver.LogEntryReceived += receiver_LogEntryReceived;
        }

        void receiver_LogEntryReceived(object sender, string e)
        {
            ShowLog(e);
        }

        async void btnStart_Click(object sender, EventArgs e)
        {
            UpdateControlStatus(LogStatus.Running);
            InitialiseReceiver();
            await receiver.Start();
        }

        async void btnStop_Click(object sender, EventArgs e)
        {
            await receiver.Stop();
            UpdateControlStatus(LogStatus.Stopped);
        }

        void btnClear_Click(object sender, EventArgs e)
        {
            listBox1.Items.Clear();
        }

        private enum LogStatus
        {
            Stopped,
            Running
        }
    }
}
