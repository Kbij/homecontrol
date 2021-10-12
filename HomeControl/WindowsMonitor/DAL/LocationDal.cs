using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SqlClient;
using MySql.Data.MySqlClient;
using System.Diagnostics;

namespace WindowsMonitor.DAL
{
    public class LocationDal
    {
        const string DB_CONN_STR = "Server=mysql.lan;Uid=hc;Pwd=bugs bunny;Database=HC_DB;";
        public Dictionary<string, GpsClient> fillLastLocation(int timeFrameHours, string clientName)
        {
            Dictionary<string, GpsClient> result = new Dictionary<string, GpsClient>();
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd;
                    if (string.IsNullOrEmpty(clientName))
                    {
                        sqlCmd = string.Format("SELECT Client.idClient, clientName, lastMessage, locationInterval, batteryLevel, latitude, longitude, accuracy, timestamp FROM Client " +
                                               " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                               " WHERE timestamp > DATE_ADD(NOW(), INTERVAL -{0} HOUR) ORDER BY timestamp", timeFrameHours);
                    }
                    else
                    {
                        sqlCmd = string.Format("SELECT Client.idClient, clientName, lastMessage, locationInterval, batteryLevel, latitude, longitude, accuracy, timestamp FROM Client " +
                                               " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                               " WHERE timestamp > DATE_ADD(NOW(), INTERVAL -{0} HOUR) and clientName = '{1}'  ORDER BY timestamp", timeFrameHours, clientName);
                    }
                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    DataTable dt = new DataTable();
                    adapter.Fill(dt); //opens and closes the DB connection automatically !! (fetches from pool)

                    foreach (DataRow dr in dt.Rows)
                    {
                        int idClient = dr.Field<int>("idClient");
                        double latitude = (double)dr.Field<decimal>("latitude");
                        double longitude = (double)dr.Field<decimal>("longitude");
                        double accuracy = dr.Field<int>("accuracy");
                        DateTime time = dr.Field<DateTime>("timestamp");
                        string name = dr.Field<string>("clientName");
                        GpsLocation location = new GpsLocation(idClient, latitude, longitude, accuracy, time);

                        if (!result.ContainsKey(name))
                        {
                            result[name] = new GpsClient(name, dr.Field<DateTime>("lastMessage"), dr.Field<int>("locationInterval"), dr.Field<int>("batteryLevel"));
                        }

                        result[name].Locations.Add(location);

                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return result;
        }

        public List<GpsLocation> GetGpsLocations(string clientName, DateTime start, DateTime end)
        {
            List<GpsLocation> result = new List<GpsLocation>();
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    MySqlCommand cmd = new MySqlCommand("SELECT clientName, latitude, longitude, accuracy, timestamp FROM Client " +
                                                        " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                                        " WHERE timestamp > @start and timestamp < @end and clientName = @client  ORDER BY timestamp", conn);
                    cmd.Parameters.AddWithValue("@start", start);
                    cmd.Parameters.AddWithValue("@end", end);
                    cmd.Parameters.AddWithValue("@client", clientName);
                    MySqlDataAdapter adapter = new MySqlDataAdapter();
                    adapter.SelectCommand = cmd;
                    DataTable dt = new DataTable();
                    adapter.Fill(dt); //opens and closes the DB connection automatically !! (fetches from pool)

                    foreach (DataRow dr in dt.Rows)
                    {
                        double latitude = (double)dr.Field<decimal>("latitude");
                        double longitude = (double)dr.Field<decimal>("longitude");
                        double accuracy = dr.Field<int>("accuracy");
                        DateTime time = dr.Field<DateTime>("timestamp");
                        GpsLocation location = new GpsLocation(latitude, longitude, accuracy, time);

                        result.Add(location);

                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
            return result;
        }

        public void deleteLocations(List<int> locations)
        {

        }

        public List<string> fillSources()
        {
            List<string> result = new List<string>();
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd = string.Format("SELECT clientName FROM Client ");
                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    DataTable dt = new DataTable();
                    adapter.Fill(dt); //opens and closes the DB connection automatically !! (fetches from pool)

                    foreach (DataRow dr in dt.Rows)
                    {
                        string clientName = dr.Field<string>("clientName");
                        result.Add(clientName);
                    }
                }
            }
            catch (Exception /*ex*/)
            {

            }
            return result;
        }
        public void updateInterval(string client, int interval)
        {
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd = string.Format("UPDATE Client SET locationInterval = {0} WHERE clientName = '{1}'", interval, client);

                    MySqlCommand cmd = new MySqlCommand(sqlCmd, conn);
                    cmd.ExecuteNonQuery();
                }
            }
            catch (Exception /*ex*/)
            {
            }
        }
    }
}
