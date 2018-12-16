using GMap.NET;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsMonitor
{
    public class GMapPoint : GMap.NET.WindowsForms.GMapMarker
    {
        private PointLatLng point_;
        private float size_;
        public PointLatLng Point
        {
            get
            {
                return point_;
            }
            set
            {
                point_ = value;
            }
        }
        public GMapPoint(PointLatLng p, int size)
            : base(p)
        {
            point_ = p;
            size_ = size;
        }

        public override void OnRender(Graphics g)
        {
            //  g.FillRectangle(Brushes.Blue, LocalPosition.X - size_/2, LocalPosition.Y - size_/2, size_, size_);
            //OR 
            System.Drawing.SolidBrush myBrush = new System.Drawing.SolidBrush(System.Drawing.Color.Blue);
            g.FillEllipse(myBrush, LocalPosition.X - size_ / 2, LocalPosition.Y - size_ / 2, size_, size_);
            //OR whatever you need
        }
    }
}
