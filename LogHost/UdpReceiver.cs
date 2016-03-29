using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Net;

namespace LogHost
{
    public class UdpReceiver
    {
        public event EventHandler<string> LogEntryReceived;

        int port;
        UdpClient client;

        public UdpReceiver(int port)
        {
            this.port = port;
        }

        public async Task Start()
        {
            LogData("Starting Log...\r\n");
            client = new UdpClient();
            // This allows us to use the same socket for testing when the client is on the same local machine
            client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            // Listening from any IP address on the required port
            client.Client.Bind(new IPEndPoint(IPAddress.Any, port));
            await Task.Run(async () => 
            {
                try
                {
                    while (true)
                    {
                        // UdpClient async methods (and the other variations) do not support CancellationTokens
                        // so as per Microsoft docs, call client.Close() to cancel and catch the exception.
                        // Dirty but can't find any other option.
                        UdpReceiveResult result = await client.ReceiveAsync();
                        string message = Encoding.ASCII.GetString(result.Buffer);
                        LogData(message);
                    }
                }
                catch (ObjectDisposedException)
                {
                    CloseClient(client);
                    LogData("Stopping Log due to user request...\r\n");
                }
                catch (Exception exc)
                {
                    CloseClient(client);
                    LogData("Stopping Log due to an error...\r\n");
                    LogData(exc.Message);
                }
            });
            
        }

        private void CloseClient(UdpClient client)
        {
            if (client != null)
            {
                client.Close();
                client = null;
            }
        }

        public async Task Stop()
        {
            // This is a fudge which is recommended by Microsoft since UdpClient.ReceiveAsync
            // does not support CancellationTokens
            await Task.Run(() => client.Close());
        }

        private void LogData(string data)
        {
            // Split multiline messages (e.g. a Stack Trace) so they are more readable
            string[] lines = data.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
            foreach (string line in lines)
                Log(line);
        }

        private void Log(string data)
        {
            if (LogEntryReceived != null)
            {
                LogEntryReceived(this, data);
            }
        }

    }
}
