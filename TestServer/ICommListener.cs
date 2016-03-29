using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    public interface ICommListener
    {
        void receiveObject(string name, Object obj);
        void logMessage(string message);
    }
}
