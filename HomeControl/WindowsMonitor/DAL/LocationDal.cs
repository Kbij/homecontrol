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
        public Dictionary<string, GpsClient> fillLastLocation(double timeFrameHours, string clientName)
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
                                               " WHERE timestamp > DATE_ADD(NOW(), INTERVAL -{0} MINUTE) ORDER BY timestamp", timeFrameHours);
                    }
                    else
                    {
                        sqlCmd = string.Format("SELECT clientName, lastMessage, locationInterval, batteryLevel, latitude, longitude, accuracy, timestamp FROM Client " +
                                               " INNER JOIN Location ON Client.idClient = Location.idClient " +
                                               " WHERE timestamp > DATE_ADD(NOW(), INTERVAL -{0} MINUTE) and clientName = '{1}'  ORDER BY timestamp", timeFrameHours, clientName);
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
                    MySqlCommand cmd = new MySqlCommand("SELECT idLocation, Client.idClient, clientName, latitude, longitude, accuracy, timestamp FROM Client " +
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
                        int idLocation = dr.Field<int>("idLocation");
                        double latitude = (double)dr.Field<decimal>("latitude");
                        double longitude = (double)dr.Field<decimal>("longitude");
                        double accuracy = dr.Field<int>("accuracy");
                        DateTime time = dr.Field<DateTime>("timestamp");
                        GpsLocation location = new GpsLocation(idLocation, latitude, longitude, accuracy, time);

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

        public void deleteLocations(List<Int32> locations)
        {
            try
            {
                var tvp = new DataTable("Location");
                //tvp.Columns.Add("idLocation", typeof(Int32));
                DataColumn idLocation = new DataColumn("idLocation", typeof(Int32));
                tvp.Columns.Add(idLocation);
                var keys = new DataColumn[1];
                keys[0] = idLocation;
                tvp.PrimaryKey = keys;
                //test.ExtendedProperties.
                foreach (var id in locations)
                {
                    DataRow newRow = tvp.NewRow();
                    newRow["idLocation"] = id;
                    tvp.Rows.Add(newRow);
                }
                tvp.AcceptChanges();
                for(int i = 0; i < tvp.Rows.Count; i++)
                {
                    tvp.Rows[i].Delete();
                }

                Console.WriteLine($"Rowsize: {tvp.Rows.Count}");
                    

                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    //MySqlCommand cmd = 

                    //using (MySqlDataAdapter da = new MySqlDataAdapter(cmd))
                    //{
                        MySqlDataAdapter da = new MySqlDataAdapter();
                        da.UpdateBatchSize = 200;
                        da.DeleteCommand = new MySqlCommand("DELETE FROM Location WHERE idLocation = @idLocation", conn);
                        da.DeleteCommand.UpdatedRowSource = UpdateRowSource.OutputParameters;

                        var parameter = da.DeleteCommand.Parameters.Add("@idLocation", MySqlDbType.Int32, 4, "idLocation");
                        parameter.SourceVersion = DataRowVersion.Original;
                          using (MySqlCommandBuilder cb = new MySqlCommandBuilder(da))
                        {
                            
                            da.Update(tvp);
                           // tran.Commit();
                        }
                 //   }

                    //cmd.Parameters.AddWithValue("@idLocation",
                    //    //cmd.Parameters.sql
                    //cmd.ExecuteNonQuery();
                    /*                    foreach(int idLocation in locations)
                                        {
                                            cmd.Parameters["@idLocation"].Value = idLocation;
                                            cmd.ExecuteNonQuery();
                                        }*/
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
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
