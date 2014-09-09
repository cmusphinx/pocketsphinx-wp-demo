using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PocketSphinxWindowsPhoneDemo.Recorder
{
    public class AudioDataEventArgs
    {
        public Byte[] Data { get; private set; }

        public AudioDataEventArgs(Byte[] data)
        {
            Data = data;
        }
    }
}
