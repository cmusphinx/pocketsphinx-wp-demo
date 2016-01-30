using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using PocketSphinxWindowsPhoneDemo.Resources;
using System.Diagnostics;
using PocketSphinxRntComp;
using PocketSphinxWindowsPhoneDemo.Recorder;
using System.Threading.Tasks;

namespace PocketSphinxWindowsPhoneDemo
{
    /// <summary>
    /// PocketSphinx implementation for Windows Phone
    /// pure code; no MVVM and all in 1 code behind file
    /// 
    /// Created by Toine de Boer, Enbyin (NL)
    /// </summary>
    public partial class MainPage : PhoneApplicationPage
    {

        #region Constant values
        
        private const string WakeupText = "oh mighty computer";

        private string[] DigitValues = new string[] { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };

        private string[] MenuValues = new string[] { "digits", "forecast" };

        #endregion

        #region Properties

        private RecognizerMode _mode;
        private RecognizerMode Mode
        {
            get { return _mode; }
            set
            {
                if(_mode != value)
                {
                    _mode = value;
                    SetRecognizerMode(_mode);
                }
            }
        }

        #endregion

        #region Fields

        private SpeechRecognizer speechRecognizer;

        private WasapiAudioRecorder audioRecorder;

        private enum RecognizerMode { Wakeup, Digits, Menu, Phones };

        private bool isPhonemeRecognitionEnabled;
               
        #endregion

        #region Constructor & Loaded event

        public MainPage()
        {
            InitializeComponent();
        }

        private void PhoneApplicationPage_Loaded(object sender, RoutedEventArgs e)
        {
            progressBar.Visibility = System.Windows.Visibility.Collapsed;
            progressBar.IsIndeterminate = false;

            RecognitionButtonsStack.Visibility = Visibility.Collapsed;
            InnitiazeButtonsStack.Visibility = Visibility.Visible;
            StateMessageBlock.Text = "Choose Recognition";
        }

        #endregion

        #region Innitial Load methods

        private async void LoadRecognitionAsync(bool phonemeRecognitionEnabled)
        {
            isPhonemeRecognitionEnabled = phonemeRecognitionEnabled;
            InnitiazeButtonsStack.Visibility = Visibility.Collapsed;

            progressBar.IsIndeterminate = true;
            ContentPanel.IsHitTestVisible = false;

            StateMessageBlock.Text = "components are loading";

            // Initializing
            await InitialzeSpeechRecognizer();
            InitializeAudioRecorder();

            // Start processes
            StartSpeechRecognizerProcessing();
            StartNativeRecorder();

            // UI            
            progressBar.Visibility = System.Windows.Visibility.Collapsed;
            progressBar.IsIndeterminate = false;
            ContentPanel.IsHitTestVisible = true;
            StateMessageBlock.Text = "ready for use";

            if (!isPhonemeRecognitionEnabled)
            {
                RecognitionButtonsStack.Visibility = Visibility.Visible;

                // Set innitial UI state
                WakeButtonOnClick(this, null);
            }
        }

        #endregion

        #region Button events

        private void InitializeNormalButtonOnClick(object sender, RoutedEventArgs e)
        {
            _mode = RecognizerMode.Wakeup;
            LoadRecognitionAsync(false);
        }

        private void InitializePhonemeButtonOnClick(object sender, RoutedEventArgs e)
        {
            _mode = RecognizerMode.Phones;
            LoadRecognitionAsync(true);
        }

        private void WakeButtonOnClick(object sender, RoutedEventArgs e)
        {
            Mode = RecognizerMode.Wakeup;
            TipMessageBlock.Text = string.Format("tip: say '{0}'", WakeupText);
        }

        private void DigitsButtonOnClick(object sender, RoutedEventArgs e)
        {
            Mode = RecognizerMode.Digits;
            TipMessageBlock.Text = string.Format("tip: say '{0}'", string.Join(",", DigitValues));
        }

        private void MenuButtonOnClick(object sender, RoutedEventArgs e)
        {
            Mode = RecognizerMode.Menu;
            TipMessageBlock.Text = string.Format("tip: say '{0}'", string.Join(",", MenuValues));
        }

        #endregion

        #region Business Logic Methods

        private void FindMatchToToggle(string recognizedText)
        {
            bool matchFound = false; 

            switch (Mode)
            {
                case RecognizerMode.Wakeup:
                    matchFound = (recognizedText == WakeupText);
                    break;
                case RecognizerMode.Digits:
                    var recognizedWords = recognizedText.Split(' ');
                    foreach (var word in recognizedWords)
                    {
                        if (DigitValues.Contains(word.ToLower()))
                        {
                            recognizedText = word;
                            matchFound = true;
                        }
                    }                    
                    break;
                case RecognizerMode.Menu:
                    matchFound = (MenuValues.Contains(recognizedText.ToLower()));
                    break;
            }

            if (matchFound)
            {
                MainMessageBlock.Text = recognizedText;
                ToggleSearch();
            }
        }

        private void ToggleSearch()
        {
            switch(Mode)
            {
                case RecognizerMode.Wakeup:
                    Mode = RecognizerMode.Digits;
                    break;
                case RecognizerMode.Digits:
                    Mode = RecognizerMode.Menu;
                    break;
                case RecognizerMode.Menu:
                    Mode = RecognizerMode.Digits;
                    break;
            }
        }

        private void FoundText(string recognizedText)
        {
            MainMessageBlock.Text = recognizedText;
        }

        private void SetRecognizerMode(RecognizerMode mode)
        {
            string result = string.Empty;
            speechRecognizer.StopProcessing();
            Debug.WriteLine(result);
            result = speechRecognizer.SetSearch(mode.ToString());
            Debug.WriteLine(result);
            speechRecognizer.StartProcessing();
            Debug.WriteLine(result);

            ModeMessageBlock.Text = string.Format("running '{0}' mode", mode);
        }

        #endregion

        #region SpeechRecognizer Methods (PocketSphinx)

        private async Task InitialzeSpeechRecognizer()
        {
            List<string> initResults = new List<string>();

            try
            {
                AudioContainer.SphinxSpeechRecognizer = new SpeechRecognizer();
                speechRecognizer = AudioContainer.SphinxSpeechRecognizer;

                speechRecognizer.resultFound += speechRecognizer_resultFound;
                speechRecognizer.resultFinalizedBySilence += speechRecognizer_resultFinalizedBySilence;

                if (!isPhonemeRecognitionEnabled)
                {
                    await Task.Run(() =>
                    {
                        var initResult = speechRecognizer.Initialize("\\Assets\\models\\hmm\\en-us", "\\Assets\\models\\dict\\cmu07a.dic");
                        initResults.Add(initResult);
                        initResult = speechRecognizer.AddKeyphraseSearch(RecognizerMode.Wakeup.ToString(), WakeupText);
                        initResults.Add(initResult);
                        initResult = speechRecognizer.AddGrammarSearch(RecognizerMode.Menu.ToString(), "\\Assets\\models\\grammar\\menu.gram");
                        initResults.Add(initResult);
                        initResult = speechRecognizer.AddGrammarSearch(RecognizerMode.Digits.ToString(), "\\Assets\\models\\grammar\\digits.gram");
                        initResults.Add(initResult);
                        initResult = speechRecognizer.AddNgramSearch("forecast", "\\Assets\\models\\lm\\weather.dmp");
                        initResults.Add(initResult);
                    });
                }
                else
                {
                    await Task.Run(() =>
                    {
                        var initResult = speechRecognizer.InitializePhonemeRecognition("\\Assets\\models\\hmm\\en-us");
                        initResults.Add(initResult);
                        initResult = speechRecognizer.AddPhonesSearch(RecognizerMode.Phones.ToString(), "\\Assets\\models\\lm\\en-us-phone.lm.bin");
                        initResults.Add(initResult);
                    });
                }

                SetRecognizerMode(Mode);
            }
            catch (Exception ex)
            {
                var initResult = ex.Message;                
                initResults.Add(initResult);
            }

            foreach (var result in initResults)
            {
                Debug.WriteLine(result);
            }
        }

        private void StartSpeechRecognizerProcessing()
        {
            string result = string.Empty;

            try
            {
                if(speechRecognizer.IsProcessing())
                {
                    result = "PocketSphinx already started";
                }
                else
                {
                    result = speechRecognizer.StartProcessing();
                }                
            }
            catch
            {
                result = "Starting PocketSphinx processing failed";
            }

            Debug.WriteLine(result);
        }

        private void StopSpeechRecognizerProcessing()
        {
            string result = string.Empty;

            try
            {
                result = speechRecognizer.StopProcessing();
            }
            catch
            {
                result = "Stopping PocketSphinx processing failed";
            }

            Debug.WriteLine(result);
        }

        void speechRecognizer_resultFinalizedBySilence(string finalResult)
        {
            Debug.WriteLine("final result found: {0}", finalResult);
            //FindMatchToToggle(finalResult);
            FoundText(finalResult);
        }

        void speechRecognizer_resultFound(string result)
        {
            Debug.WriteLine("result found: {0}", result);
            StateMessageBlock.Text = string.Format("found: {0}", result);
        }

        #endregion

        #region Recording Methods

        private void InitializeAudioRecorder()
        {
            AudioContainer.AudioRecorder = new WasapiAudioRecorder();
            audioRecorder = AudioContainer.AudioRecorder;
            audioRecorder.BufferReady += audioRecorder_BufferReady;

            Debug.WriteLine("AudioRecorder Initialized");
        }

        private void StartNativeRecorder()
        {
            audioRecorder.StartRecording();
        }

        private void StopNativeRecorder()
        {
            audioRecorder.StopRecording();
        }

        void audioRecorder_BufferReady(object sender, AudioDataEventArgs e)
        {
            int registerResult = 0;
            try
            {
                registerResult = speechRecognizer.RegisterAudioBytes(e.Data);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                StopNativeRecorder();
                StopSpeechRecognizerProcessing();
                StateMessageBlock.Text = "all stoped because of error";
            }

            // incoming raw sound
            //Debug.WriteLine("{0} bytes of raw audio recieved, {1} frames processed at PocketSphinx", e.Data.Length, registerResult);
        }

        #endregion
    }
}