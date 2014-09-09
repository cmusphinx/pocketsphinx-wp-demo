#pragma once

namespace PocketSphinxRntComp
{
	public delegate void ResultFoundHandler(Platform::String^ result);

	public ref class SpeechRecognizer sealed
    {
    public:
		SpeechRecognizer();

		Platform::String^ Initialize(Platform::String^ hmmFilePath, Platform::String^ lmFilePath, Platform::String^ dictFilePath);
		Platform::String^ AddKeyphraseSearch(Platform::String^ name, Platform::String^ keyphrase);
		Platform::String^ AddGrammarSearch(Platform::String^ name, Platform::String^ filePath);
		Platform::String^ AddNgramSearch(Platform::String^ name, Platform::String^ filePath);
		Platform::String^ SetSearch(Platform::String^ name);
		Platform::String^ StartProcessing(void);
		Platform::String^ StopProcessing(void);
		Platform::String^ RestartProcessing(void);

		Platform::String^ TestPocketSphinx(void);
		Platform::String^ CleanPocketSphinx(void);

		int SpeechRecognizer::RegisterAudioBytes(const Platform::Array<uint8>^ audioBytes);
				
		event ResultFoundHandler^ resultFound;
		event ResultFoundHandler^ resultFinalizedBySilence;

	private:
		void SpeechRecognizer::OnResultFound(Platform::String^ result);
		void SpeechRecognizer::OnResultFinalizedBySilence(Platform::String^ finalResult);

    };
}