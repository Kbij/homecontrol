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
using Android.Graphics;

namespace HomeControl.Logic
{
    public class MyViewHolder : Java.Lang.Object
    {
        public TextView Name { get; set; }
        public TextView Temperature { get; set; }
    }
    public class TemperatureAdapter : BaseAdapter, ITemperatureListener
    {
        Activity mActivity;
        TemperatureModel mModel;
        Handler mHandler;
        List<RoomTemperature> mRoomTemperatues;
        public TemperatureAdapter(Activity activity, TemperatureModel model)
        {
            mActivity = activity;
            mModel = model;
            mHandler = new Handler();
            model.registerListener(this);
            mRoomTemperatues = mModel.roomTemperatues();
        }

        public void refresh()
        {
            mRoomTemperatues = mModel.roomTemperatues();
            mActivity.RunOnUiThread(new Action(() => {
                base.NotifyDataSetChanged();
            }));
        
        }

        public override int Count
        {
            get { return mRoomTemperatues.Count; }
            // get { return 10; }
        }

        public override Java.Lang.Object GetItem(int position)
        {
            return null;
        }

        public override long GetItemId(int position)
        {
            return 0;
        }

        // create a new ImageView for each item referenced by the Adapter
        public override View GetView(int position, View convertView, ViewGroup parent)
        {
            MyViewHolder holder = null;

            LayoutInflater inflater = mActivity.LayoutInflater;

            if (convertView == null)
            {
                holder = new MyViewHolder();
                convertView = inflater.Inflate(Resource.Layout.TemperatureItem, parent, false);

                holder.Name = (TextView)convertView.FindViewById(Resource.Id.textName);
                holder.Temperature = (TextView)convertView.FindViewById(Resource.Id.textTemperature);

                Typeface myTypeface = Typeface.CreateFromAsset(mActivity.Assets, "DS-DIGIB.TTF");
                holder.Temperature.Typeface = myTypeface;

                convertView.Tag = holder;
            }
            else
            {
                holder = (MyViewHolder)convertView.Tag;
            }

            holder.Name.Text = mRoomTemperatues[position].name;
            holder.Temperature.Text = string.Format("{0:0.0} °C", mRoomTemperatues[position].roomTemperature);

            return convertView;
        }
    }
}