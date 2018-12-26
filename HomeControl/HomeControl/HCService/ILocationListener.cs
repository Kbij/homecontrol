﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.Locations;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;

namespace HomeControl.HCService
{
    public interface ILocationListener
    {
        void NewLocation(Location location, double distanceTraveled);
    }
}