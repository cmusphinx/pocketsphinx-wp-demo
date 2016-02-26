using System;
using System.Windows.Threading;
using Microsoft.Xna.Framework;
using PocketSphinxRntComp;

namespace PocketSphinxWindowsPhoneDemo.Recorder
{
    /// <summary>
    /// Audio Recorder working with Native WASAPI audio recording 
    /// (less interruptions by UI thread then C# recorder)
    /// </summary>
    public class WasapiAudioRecorder
    {
        private readonly WasapiAudio wasapiAudio;
        private bool wasapiIsEnabled;
        private readonly DispatcherTimer retrieveAudioTimer;

        public event EventHandler<AudioDataEventArgs> AudioReported;

        public WasapiAudioRecorder()
        {
            wasapiAudio = new WasapiAudio();

            retrieveAudioTimer = new DispatcherTimer();
            retrieveAudioTimer.Interval = TimeSpan.FromMilliseconds(33);
            retrieveAudioTimer.Tick += RetrieveAudio;
        }

        public void StartRecording()
        {
            wasapiIsEnabled = true;
            wasapiAudio.StartAudioCapture();
            retrieveAudioTimer.Start();
        }

        public void StopRecording()
        {
            wasapiIsEnabled = false;
            wasapiAudio.StopAudioCapture();
            retrieveAudioTimer.Stop();
        }

        private void RetrieveAudio(object sender, EventArgs e)
        {
            try { FrameworkDispatcher.Update(); }
            catch
            {
                // ignored
            }

            if (!wasapiIsEnabled) return;

            // XnaAudio stores the buffer in callback method.
            // Buffer is retrieved manually when recording using WASAPI.
            byte[] bytes;
            var size = wasapiAudio.ReadBytes(out bytes);

            if (size > 0)
            {
                OnAudioReported(bytes);
            }
        }

        private void OnAudioReported(byte[] data)
        {
            if (AudioReported != null)
            {
                AudioReported(this, new AudioDataEventArgs(data));
            }
        }
    }
}
