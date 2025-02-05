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
        private const int MAX_MINUTES = 30;
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
            LocationFilter filter = new LocationFilter(MIN_DISTANCE, MAX_MINUTES);
            List<int> deleteLocations = filter.filterLocations(locations);
            mDb.deleteLocations(deleteLocations);
        }


    }
}
