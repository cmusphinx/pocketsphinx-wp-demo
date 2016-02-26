#pragma once

/**
* Created by Toine de Boer, Enbyin (NL)
* 
* Intended as kick-start using PocketSphinx on Windows mobile platforms
* Methods are listed in order of usage
*/

namespace PocketSphinxRntComp
{
	public delegate void ResultFoundHandler(Platform::String^ result);

	public value struct NbestHypotheses
	{
		Platform::String^ FinalHypothesis;
		int32 FinalHypothesisScore;
		Platform::String^ HypothesesAndScores;
	};

	public ref class SpeechRecognizer sealed
    {
    public:
		SpeechRecognizer();

		// STEP 1: Initialize (pick one)
		Platform::String^ Initialize(Platform::String^ hmmFilePath, Platform::String^ dictFilePath);
		Platform::String^ InitializePhonemeRecognition(Platform::String^ hmmFilePath);

		// STEP 2: Load search or multiple searches (currently Phone search needs to work solely)
		Platform::String^ AddKeyphraseSearch(Platform::String^ searchName, Platform::String^ keyphrase);
		Platform::String^ AddGrammarSearch(Platform::String^ searchName, Platform::String^ filePath);
		Platform::String^ AddNgramSearch(Platform::String^ searchName, Platform::String^ filePath);
		Platform::String^ AddPhonesSearch(Platform::String^ searchName, Platform::String^ filePath);
		
		// STEP 3: Set search
		Platform::String^ SetSearch(Platform::String^ searchName);

		///
		/// X: Continuous recognition (get result by events)
		///

		// STEP 4.X: Start processing
		Platform::String^ StartProcessing(void);
		Platform::String^ StopProcessing(void);
		Platform::String^ RestartProcessing(void);
		Platform::Boolean IsProcessing(void);
		
		// STEP 5.X: Register incomming audio
		int SpeechRecognizer::RegisterAudioBytes(const Platform::Array<uint8>^ audioBytes);	
				
		// STEP 6.X: Wait for results
		event ResultFoundHandler^ resultFound;
		event ResultFoundHandler^ resultFinalizedBySilence;

		///
		/// 4.Y: Single utterance recognition (get instant result)
		///
		Platform::String^
			SpeechRecognizer::GetHypothesisFromUtterance(const Platform::Array<uint8>^ audioBytes);
		NbestHypotheses
			SpeechRecognizer::GetNbestFromUtterance(const Platform::Array<uint8>^ audioBytes, int32 maximumNBestIterations);


		// Extra
		Platform::String^ CleanPocketSphinx(void); // Clean before you leave

	private:
		void SpeechRecognizer::OnResultFound(Platform::String^ result);
		void SpeechRecognizer::OnResultFinalizedBySilence(Platform::String^ finalResult);
		Platform::Boolean IsReadyForProcessing(Platform::String^& message);
    };
}