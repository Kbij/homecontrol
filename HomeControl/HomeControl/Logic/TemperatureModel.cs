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
using HomeControl.Comm;
using HomeControl.HCService;

namespace HomeControl.Logic
{
    public class RoomTemperature
    {
        public string roomId;
        public string name;
        public double roomTemperature;
        public double setTemperature;
        public RoomTemperature()
        {
            roomId = "";
            name = "";
            roomTemperature = 0;
            setTemperature = 0;
        }

        public RoomTemperature(RoomTemperature room)
        {
            this.roomId = room.roomId;
            this.name = room.name;
            this.roomTemperature = room.roomTemperature;
            this.setTemperature = room.setTemperature;
        }
    }

    public class TemperatureModel : ICommReceiver
    {
        private List<Logic.RoomTemperature> mRoomTemperatures;
        private List<ITemperatureListener> mListeners;
        private HomeControlService mCommService;
        public TemperatureModel()
        {
            mRoomTemperatures = new List<RoomTemperature>();
            mListeners = new List<ITemperatureListener>();
            mCommService = null;
        }

        public void registerListener(ITemperatureListener listener)
        {
            mListeners.Add(listener);
        }

        public void unRegisterListener(ITemperatureListener listener)
        {
            mListeners.Add(listener);
        }

        public void setCommService(HomeControlService service)
        {
            mCommService = service;
            TemperatureMonitoring monitoring = new TemperatureMonitoring();
            monitoring.monitoringEnabled = true;
            mCommService.sendObject(monitoring);
        }

        public void unsetCommService()
        {
            TemperatureMonitoring monitoring = new TemperatureMonitoring();
            monitoring.monitoringEnabled = false;
            mCommService.sendObject(monitoring);
            mCommService = null;
        }

        public void receiveObject(object obj)
        {
            lock (mRoomTemperatures)
            {
                RoomList roomList = obj as RoomList;
                if (roomList != null)
                {
                    mRoomTemperatures.Clear();
                    if (roomList.rooms != null)
                    {
                        foreach (var commRoom in roomList.rooms)
                        {
                            Logic.RoomTemperature room = new RoomTemperature();
                            room.roomId = commRoom.roomId;
                            room.name = commRoom.name;
                            room.roomTemperature = commRoom.roomTemperature;
                            room.setTemperature = commRoom.setTemperature;
                            mRoomTemperatures.Add(room);
                        }
                    }
                }
                if ((obj as Comm.RoomTemperature) != null)
                {
                    foreach (var room in mRoomTemperatures)
                    {
                        if (room.roomId == ((Comm.RoomTemperature)obj).roomId)
                        {
                            room.roomTemperature = ((Comm.RoomTemperature)obj).roomTemperature;
                        }
                    }
                }
            }

            foreach(var listener in mListeners)
            {
                listener.refresh();
            }
        }

        public void connected()
        {
            if (mCommService != null)
            {
                TemperatureMonitoring monitoring = new TemperatureMonitoring();
                monitoring.monitoringEnabled = true;
                mCommService.sendObject(monitoring);
            }
        }

        public void disconnected()
        {
            lock (mRoomTemperatures)
            {
                mRoomTemperatures.Clear();
                foreach (var listener in mListeners)
                {
                    listener.refresh();
                }
            }
        }
        public List<RoomTemperature> roomTemperatues()
        {
            lock (mRoomTemperatures)
            {
                return new List<RoomTemperature>(mRoomTemperatures);
            }
        }

        public RoomTemperature roomTemperature(string roomId)
        {
            lock (mRoomTemperatures)
            {
                foreach (var room in mRoomTemperatures)
                {
                    if (room.roomId == roomId)
                    {
                        return new RoomTemperature(room);
                    }
                }
            }
            return null;
        }


    }
}