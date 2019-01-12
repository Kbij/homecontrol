using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SqlClient;
using MySql.Data.MySqlClient;

namespace WindowsMonitor.DAL
{
    public class LocationDal
    {
        const string DB_CONN_STR = "Server=192.168.10.7;Uid=hc;Pwd=bugs bunny;Database=HC_DB;";
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
                        sqlCmd = string.Format("SELECT clientName, lastMessage, locationInterval, batteryLevel, latitude, longitude, accuracy, timestamp FROM Client " +
                                               " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                               " WHERE TIMESTAMPDIFF(HOUR, timestamp, NOW()) < {0} ORDER BY timestamp", timeFrameHours);
                    }
                    else
                    {
                        sqlCmd = string.Format("SELECT clientName, lastMessage, locationInterval, batteryLevel, latitude, longitude, accuracy, timestamp FROM Client " +
                                               " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                               " WHERE TIMESTAMPDIFF(HOUR, timestamp, NOW()) < {0} and clientName = '{1}'  ORDER BY timestamp", timeFrameHours, clientName);
                    }
                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    DataTable dt = new DataTable();
                    adapter.Fill(dt); //opens and closes the DB connection automatically !! (fetches from pool)

                    foreach (DataRow dr in dt.Rows)
                    {
                        double latitude = (double)dr.Field<decimal>("latitude");
                        double longitude = (double)dr.Field<decimal>("longitude");
                        double accuracy = dr.Field<int>("accuracy");
                        DateTime time = dr.Field<DateTime>("timestamp");
                        string name = dr.Field<string>("clientName");
                        GpsLocation location = new GpsLocation(latitude, longitude, accuracy, time);

                        if (!result.ContainsKey(name))
                        {
                            result[name] = new GpsClient(name, dr.Field<DateTime>("lastMessage"), dr.Field<int>("locationInterval"), dr.Field<int>("batteryLevel"));
                        }

                        result[name].Locations.Add(location);

                    }
                }
            }
            catch (Exception /*ex*/)
            {

            }
            return result;
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
            catch (Exception ex)
            {

            }
        }
    }
}
