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

	public ref class SpeechRecognizer sealed
    {
    public:
		SpeechRecognizer();

		// STEP 1: Initialize and Load searches
		Platform::String^ Initialize(Platform::String^ hmmFilePath, Platform::String^ dictFilePath);
		Platform::String^ AddKeyphraseSearch(Platform::String^ name, Platform::String^ keyphrase);
		Platform::String^ AddGrammarSearch(Platform::String^ name, Platform::String^ filePath);
		Platform::String^ AddNgramSearch(Platform::String^ name, Platform::String^ filePath);
		
		// STEP 2: Set search
		Platform::String^ SetSearch(Platform::String^ name);		

		// STEP 3: Start processing
		Platform::String^ StartProcessing(void);
		Platform::String^ StopProcessing(void);
		Platform::String^ RestartProcessing(void);
		Platform::Boolean IsProcessing(void);
		
		// STEP 4: Register incomming audio
		int SpeechRecognizer::RegisterAudioBytes(const Platform::Array<uint8>^ audioBytes);	
				
		// STEP 5: Wait for results
		event ResultFoundHandler^ resultFound;
		event ResultFoundHandler^ resultFinalizedBySilence;
				
		Platform::String^ CleanPocketSphinx(void);

		// Test method (to use a raw recorded audio file)
		Platform::String^ TestPocketSphinx(void);

	private:
		void SpeechRecognizer::OnResultFound(Platform::String^ result);
		void SpeechRecognizer::OnResultFinalizedBySilence(Platform::String^ finalResult);
		Platform::Boolean IsReadyForProcessing(Platform::String^& message);
    };
}