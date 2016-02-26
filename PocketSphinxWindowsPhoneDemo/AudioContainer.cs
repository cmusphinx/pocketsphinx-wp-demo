using System.Diagnostics;
using PocketSphinxRntComp;
using PocketSphinxWindowsPhoneDemo.Recorder;

namespace PocketSphinxWindowsPhoneDemo
{
    public static class AudioContainer
    {

        /// <summary>
        /// PocketSphinx speech recognizer in a Runtime Component
        /// </summary>
        public static SpeechRecognizer SphinxSpeechRecognizer
        { get; set; }

        /// <summary>
        /// Native Audio Recorder working with WASAPI
        /// </summary>
        public static WasapiAudioRecorder AudioRecorder
        { get; set; }

        /// <summary>
        /// Stop all
        /// </summary>
        public static void StopAllAudioProccesses()
        {
            if (AudioRecorder != null)
            {
                AudioRecorder.StopRecording();
            }

            if (SphinxSpeechRecognizer != null)
            {
                var result = string.Empty;
                result = SphinxSpeechRecognizer.StopProcessing();
                Debug.WriteLine(result);
            }
        }

        /// <summary>
        /// Dispose
        /// </summary>
        public static void Dispose()
        {
            if (SphinxSpeechRecognizer != null)
            {
                var cleanResult = string.Empty;
                cleanResult = SphinxSpeechRecognizer.CleanPocketSphinx();
                Debug.WriteLine(cleanResult);
            }
        }

    }
}
