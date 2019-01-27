using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using HomeControl.Logic;

namespace HomeControl
{
    [Activity(Label = "RoomDetail", ParentActivity = typeof(MainActivity))]
    public class RoomDetail : Activity, ITemperatureListener
    {
        private string mRoomId;
        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);
            SetContentView(Resource.Layout.ChangeTemperature);
            mRoomId = Intent.GetStringExtra("roomId");

            var room = TemperatureModel.Instance.roomTemperature(mRoomId);
            if (room != null)
            {
                Window.SetTitle(room.name);
                var detailTextView = FindViewById<TextView>(Resource.Id.setTemperature);
                var buttonUp = FindViewById<Button>(Resource.Id.temperatureUp);
                buttonUp.Click += ButtonUp_Click;
                var buttonDown = FindViewById<Button>(Resource.Id.temperatureDown);
                buttonDown.Click += ButtonDown_Click; ;

                detailTextView.Text = string.Format("{0:0.0} °C", TemperatureModel.Instance.roomTemperature(mRoomId).setTemperature);
                TemperatureModel.Instance.registerListener(this);
            }
            else
            {
                Window.SetTitle("Unknown room");
            }
        }

        protected override void OnStop()
        {
            TemperatureModel.Instance.unRegisterListener(this);
            base.OnStop();
        }

        private void ButtonDown_Click(object sender, EventArgs e)
        {
            TemperatureModel.Instance.setTemperatureDown(mRoomId);
        }

        private void ButtonUp_Click(object sender, EventArgs e)
        {
            TemperatureModel.Instance.setTemperatureUp(mRoomId);
        }

        public void refresh()
        {
            var detailTextView = FindViewById<TextView>(Resource.Id.setTemperature);
            if (detailTextView != null)
            {
                var room = TemperatureModel.Instance.roomTemperature(mRoomId);
                if (room != null)
                {
                    detailTextView.Text = string.Format("{0:0.0} °C", room.setTemperature);
                }
            }
        }
    }
}