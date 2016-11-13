using MySql.Data.MySqlClient;
using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor.DAL
{
    public class TemperatureDal
    {
        private const string DB_CONN_STR = "Server=192.168.10.7;Uid=hc;Pwd=bugs bunny;Database=HC_DB;";
        public void fillTemperatures(HomeControlData.TemperatureDataTable dt, DateTime start, DateTime end, string roomId)
        {
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd = "SELECT * FROM HC_DB.Temperature " +
                                    " INNER JOIN TemperatureSensor on Temperature.idSensor = TemperatureSensor.idTemperatureSensor " + 
                                    " INNER JOIN Room ON TemperatureSensor.idRoom = Room.idRoom" +
                                    " WHERE date > @start and date < @end and RoomId = @roomId";

                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    adapter.SelectCommand.Parameters.AddWithValue("@start", start);
                    adapter.SelectCommand.Parameters.AddWithValue("@end", end);
                    adapter.SelectCommand.Parameters.AddWithValue("@roomId", roomId);
                    adapter.Fill(dt);
                }
            }
            catch (Exception ex)
            {

            }
        }

        public void fillRooms(HomeControlData.RoomDataTable dt)
        {
            try
            {
                using (MySqlConnection conn = new MySqlConnection(DB_CONN_STR))
                {
                    conn.Open();
                    string sqlCmd = "SELECT * FROM HC_DB.Room";

                    MySqlDataAdapter adapter = new MySqlDataAdapter(sqlCmd, conn);
                    adapter.SelectCommand.CommandType = CommandType.Text;
                    adapter.Fill(dt);
                }
            }
            catch (Exception ex)
            {

            }
        }
    }
}
