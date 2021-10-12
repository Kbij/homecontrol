using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsMonitor.DAL;

namespace WindowsMonitor
{
    public partial class DataCleanup : Form
    {
        private LocationDal mDb;
        private const int MIN_DISTANCE = 50;
        public DataCleanup()
        {
            InitializeComponent();
            mDb = new LocationDal();
            cmbClient.DataSource = mDb.fillSources();
        }

        private void btnGo_Click(object sender, EventArgs e)
        {
            DateTime start = dtSelectedDate.Value.AddDays(-1);
            DateTime end = dtSelectedDate.Value;
            List<GpsLocation> locations = mDb.GetGpsLocations(cmbClient.SelectedItem.ToString(), start, end);

            List<int> deleteLocations = filterLocations(locations);
            mDb.deleteLocations(deleteLocations);
        }

        private List<int> filterLocations(List<GpsLocation> locations)
        {
            List<int> result = new List<int>();
            GpsLocation previous = null;
            foreach (var location in locations)
            {
                if (previous != null)
                {
                    double distance = location.distance(previous);
                    if (location.distance(previous) < 10)
                    {
                        result.Add(location.Id);
                    }
                    else
                    {
                        previous = location;
                    }

                }
                else
                {
                    previous = location;
                }
            }

            return result;
        }
    }
}
