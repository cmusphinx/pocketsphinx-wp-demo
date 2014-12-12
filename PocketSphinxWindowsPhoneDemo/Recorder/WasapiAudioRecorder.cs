using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;
using PocketSphinxRntComp;

namespace PocketSphinxWindowsPhoneDemo.Recorder
{
    /// <summary>
    /// Native Audio Recorder working with WASAPI
    /// </summary>
    public class WasapiAudioRecorder
    {

        #region Fields

        private WasapiAudio wasapiAudio;

        private bool wasapiIsEnabled;

        private DispatcherTimer intervalTimer;


        #endregion

        #region Constructor

        public WasapiAudioRecorder()
        {
            wasapiAudio = new WasapiAudio();

            intervalTimer = new DispatcherTimer();
            intervalTimer.Interval = TimeSpan.FromMilliseconds(33);
            intervalTimer.Tick += new EventHandler(intervalTimer_Tick);
        }

        #endregion

        #region Public Methods

        public void StartRecording()
        {
            wasapiIsEnabled = true;
            wasapiAudio.StartAudioCapture();
            intervalTimer.Start();
        }

        public void StopRecording()
        {
            wasapiIsEnabled = false;
            wasapiAudio.StopAudioCapture();
            intervalTimer.Stop();
        }

        #endregion

        #region Private Methods

        private void OnBufferReady(byte[] data)
        {
            if (BufferReady != null)
            {
                BufferReady(this, new AudioDataEventArgs(data));
            }
        }

        #endregion

        #region Events

        public event EventHandler<AudioDataEventArgs> BufferReady;

        #endregion

        #region Event Handlers

        void intervalTimer_Tick(object sender, EventArgs e)
        {
            try { FrameworkDispatcher.Update(); }
            catch { }

            if (wasapiIsEnabled)
            {
                // XnaAudio stores the buffer in callback method.
                // Buffer is retrieved manually when recording using WASAPI.
                byte[] bytes = null;
                int size = wasapiAudio.ReadBytes(out bytes);

                if (size > 0)
                {
                    OnBufferReady(bytes);
                }
            }
        }

        #endregion
    }
}
