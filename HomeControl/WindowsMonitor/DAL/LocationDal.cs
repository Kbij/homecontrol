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
        public Dictionary<Tuple<string, DateTime>, List<GpsLocation>> fillLastLocation(int timeFrameHours)
        {
            Dictionary<Tuple<string, DateTime>, List<GpsLocation>> result = new Dictionary<Tuple<string, DateTime>, List<GpsLocation>>();

            //const string DB_CONN_STR = "Server=192.168.10.142;Uid=hc;Pwd=bugs bunny;Database=HC_DB;";
            const string DB_CONN_STR = "Server=192.168.10.7;Uid=hc;Pwd=bugs bunny;Database=HC_DB;";
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd = string.Format("SELECT clientName, lastMessage, latitude, longitude, accuracy, timestamp FROM Client " +
                                                  " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                                  " WHERE TIMESTAMPDIFF(HOUR, timestamp, NOW()) < {0}  ORDER BY timestamp", timeFrameHours);
                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    DataTable dt = new DataTable();
                    adapter.Fill(dt); //opens and closes the DB connection automatically !! (fetches from pool)

                    foreach (DataRow dr in dt.Rows)
                    {
                        double latitude = (double)dr.Field<decimal>("latitude");
                        double longitude = (double)dr.Field<decimal>("longitude");
                        double accuracy = dr.Field<int>("accuracy");
                        string client = dr.Field<string>("clientName");
                        DateTime time = dr.Field<DateTime>("timestamp");
                        GpsLocation location = new GpsLocation(latitude, longitude, accuracy, time);

                        Tuple<string, DateTime> clientTuple = new Tuple<string, DateTime>(dr.Field<string>("clientName"), dr.Field<DateTime>("lastMessage"));
                        if (!result.ContainsKey(clientTuple))
                        {
                            result[clientTuple] = new List<GpsLocation>();
                        }

                        result[clientTuple].Add(location);

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
