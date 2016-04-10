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
        public Dictionary<string, List<GpsLocation>> fillLastLocation(int timeFrameHours)
        {
            Dictionary<string, List<GpsLocation>> result = new Dictionary<string, List<GpsLocation>>();

            const string DB_CONN_STR = "Server=192.168.10.142;Uid=hc;Pwd=bugs bunny;Database=HC_DB;";
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd = string.Format("SELECT client, latitude, longitude, accuracy, timestamp FROM Locations WHERE latitude IS NOT NULL " +
                                    "  AND TIMESTAMPDIFF(HOUR, timestamp, NOW()) < {0}  ORDER BY timestamp", 6);
                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    DataTable dt = new DataTable();
                    adapter.Fill(dt); //opens and closes the DB connection automatically !! (fetches from pool)

                    foreach (DataRow dr in dt.Rows)
                    {
                        double latitude = (double)dr.Field<decimal>("latitude");
                        double longitude = (double)dr.Field<decimal>("longitude");
                        double accuracy = dr.Field<int>("accuracy");
                        string client = dr.Field<string>("client");
                        DateTime time = dr.Field<DateTime>("timestamp");
                        GpsLocation location = new GpsLocation(latitude, longitude, accuracy, time);

                        if (!result.ContainsKey(client))
                        {
                            result[client] = new List<GpsLocation>();
                        }

                        result[dr.Field<string>("client")].Add(location);

                        //                    Console.WriteLine(string.Format("user_id = {0}", dr["latitude"].ToString()));
                    }
                }
            }
            catch (Exception ex)
            {

            }
            return result;
        }
    }
}
