namespace HomeControl.Comm
{
    public interface ICommObject
    {
        byte objectId();
        string serialise();
        void deserialise(string json);
    }
}