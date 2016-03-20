namespace HomeControl.Comm
{
    public interface ICommReceiver
    {
        void receiveObject(object obj);
        void connected();
        void disconnected();
    }
}