/**
* Created by Toine de Boer, Enbyin (NL)
*
* intended as kick-start using PocketSphinx on Windows mobile platforms
*/

//#include <collection.h>
//#include <algorithm>
//using namespace Platform::Collections;
//using namespace Windows::Foundation::Collections;

#include "SpeechRecognizer.h"

using namespace PocketSphinxRntComp;
using namespace Platform;

#include <pocketsphinx.h>
#include <sphinxbase/err.h>
#include <sphinxbase/jsgf.h>

#include "Output.h"

#pragma region Info

// using pocketsphinx demo: http://cmusphinx.sourceforge.net/wiki/tutorialpocketsphinx
// api pocketsphinx: http://cmusphinx.sourceforge.net/doc/pocketsphinx/
// reference  http://blog.csdn.net/zouxy09/article/details/7978108

/// a litle part from the Android version:

//private static final String KWS_SEARCH = "wakeup";
//private static final String FORECAST_SEARCH = "forecast";
//private static final String DIGITS_SEARCH = "digits";
//private static final String MENU_SEARCH = "menu";
//private static final String KEYPHRASE = "oh mighty computer";

//// Create grammar-based searches.
//File menuGrammar = new File(modelsDir, "grammar/menu.gram");
//recognizer.addGrammarSearch(MENU_SEARCH, menuGrammar);
//File digitsGrammar = new File(modelsDir, "grammar/digits.gram");
//recognizer.addGrammarSearch(DIGITS_SEARCH, digitsGrammar);
//// Create language model search.
//File languageModel = new File(modelsDir, "lm/weather.dmp");
//recognizer.addNgramSearch(FORECAST_SEARCH, languageModel);

//No new — no delete.In the same way, no malloc(or calloc or realloc) — no free. Isn't that simple?

#pragma endregion



#pragma region Private Fields

enum RecognitionType { None, Grammatic, Phonemes };
RecognitionType initializedRecognitionType = RecognitionType::None;

ps_decoder_t *decoder;

bool isSearchSet = false;
bool isInSpeech = false;
bool isProcessing = false;

Platform::String^ lastHypothesis = "";

char applicationInstallFolderPath[1024];
char applicationLocalStorageFolder[1024];

#pragma endregion

#pragma region Constructor

SpeechRecognizer::SpeechRecognizer()
{
}

#pragma endregion

#pragma region Static Helpers

// Dont forget to free memory after usage "result" output
static char* concat(char *s1, char *s2)
{
	char *result = (char *)malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

// Dont forget to free memory after usage "characters" output
static char* convertStringToChars(Platform::String^ input)
{
	const wchar_t *platformCharacters = input->Data();

	int Size = wcslen(platformCharacters);
	char *characters = new char[Size + 1];
	characters[Size] = 0;
	for (int i = 0; i < Size; i++)
	{
		characters[i] = (char)platformCharacters[i];
	}

	return characters; 
}

String^ convertCharsToString(const char* chars)
{
	if (chars == NULL)
	{
		return ref new Platform::String();
	}
	
	static wchar_t buffer[1024];
	mbstowcs(buffer, chars, 1024);
	return ref new Platform::String(buffer);
}

#pragma endregion

#pragma region Initialize methods

//// Initialize decoder with default models
/// hmmFolderPath	== the acoustic model folder (content folder: http://cmusphinx.sourceforge.net/wiki/tutorialam#using_the_model)
/// dictFilePath	== the dictionary file
Platform::String^ SpeechRecognizer::Initialize(Platform::String^ hmmFilePath, Platform::String^ dictFilePath)
{
	if (initializedRecognitionType != RecognitionType::None)
	{
		return "PocketSphinx is already initialized";
	}

	cmd_ln_t *config;

	// Get Local Storage Path
	wcstombs(applicationLocalStorageFolder, Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data(), 1024);
	// Get Installed Folder path
	wcstombs(applicationInstallFolderPath, Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data(), 1024);

	// Get Error file path (optionaly)
	char *logPath = concat(applicationLocalStorageFolder, "\\errors.log");

	// Create full hmm and dict file paths
	auto ChmmFilePath = convertStringToChars(hmmFilePath);
	auto CdictFilePath = convertStringToChars(dictFilePath);
	char *hmmPath = concat(applicationInstallFolderPath, ChmmFilePath);
	char *dictPath = concat(applicationInstallFolderPath, CdictFilePath);

	// Create decoder config
	config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", hmmPath,
		"-dict", dictPath,
		"-mmap", "no",
		"-logfn", logPath,
		"-kws_threshold", "1e-40",
		NULL);

	// Cleanup
	free(ChmmFilePath);
	free(CdictFilePath);
	free(logPath);
	free(hmmPath);
	free(dictPath);

	if (config == NULL)
		return "Could not create a config";

	decoder = ps_init(config);
	if (decoder == NULL)
		return "Could not create a decoder";

	initializedRecognitionType = RecognitionType::Grammatic;
	return "PocketSphinx initialized";
}

// Initialize decoder with default models
//hmmFolderPath	== the acoustic model folder
Platform::String^ SpeechRecognizer::InitializePhonemeRecognition(Platform::String^ hmmFilePath)
{
	if (initializedRecognitionType != RecognitionType::None)
	{
		return "PocketSphinx is already initialized";
	}

	cmd_ln_t *config;

	// Get Local Storage Path
	wcstombs(applicationLocalStorageFolder, Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data(), 1024);
	// Get Installed Folder path
	wcstombs(applicationInstallFolderPath, Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data(), 1024);

	// Get Error file path (optionaly)
	char *logPath = concat(applicationLocalStorageFolder, "\\errors.log");

	// Create full hmm file path
	auto ChmmFilePath = convertStringToChars(hmmFilePath);
	char *hmmPath = concat(applicationInstallFolderPath, ChmmFilePath);

	// Create decoder config
	config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", hmmPath,
		"-logfn", logPath,
		"-allphone_ci", "yes",
		"-lw", "2.0",
		"-beam", "1e-20",
		"-pbeam", "1e-20",
		NULL);

	free(ChmmFilePath);
	free(logPath);
	free(hmmPath);

	if (config == NULL)
		return "Could not create a config";

	decoder = ps_init(config);
	if (decoder == NULL)
		return "Could not create a decoder";

	initializedRecognitionType = RecognitionType::Phonemes;
	return "PocketSphinx initialized";
}

#pragma endregion

#pragma region Load Methods

//// Add keyword-activation search
Platform::String^ SpeechRecognizer::AddKeyphraseSearch(Platform::String^ searchName, Platform::String^ keyphrase)
{
	if (initializedRecognitionType != RecognitionType::Grammatic)
	{
		return "Error: This type of decoding is not initialized";
	}

	char *Cname = convertStringToChars(searchName);
	char *Ckeyphrase = convertStringToChars(keyphrase);

	int result = ps_set_keyphrase(decoder, Cname, Ckeyphrase);

	free(Cname);
	free(Ckeyphrase);
	
	return (result == 0) ?
		Platform::String::Concat(searchName, " keyphrase search added") :
		Platform::String::Concat("fault adding keyphrase search: ", searchName);
}

//// Add grammar-based searches (like .jsgf or .gram files)
Platform::String^ SpeechRecognizer::AddGrammarSearch(Platform::String^ searchName, Platform::String^ filePath)
{
	if (initializedRecognitionType != RecognitionType::Grammatic)
	{
		return "Error: This type of decoding is not initialized";
	}

	char *Cname = convertStringToChars(searchName);
	char *CfilePath = convertStringToChars(filePath);
	char *CcompleteFilePath = concat(applicationInstallFolderPath, CfilePath);

	int result = ps_set_jsgf_file(decoder, Cname, CcompleteFilePath);

	free(Cname);
	free(CfilePath);
	free(CcompleteFilePath);

	return (result == 0)? 
		Platform::String::Concat(searchName, " grammar search added") :
		Platform::String::Concat("fault adding grammar search: ", searchName);
}

//// Add language model search (like .dmp files)
Platform::String^ SpeechRecognizer::AddNgramSearch(Platform::String^ searchName, Platform::String^ filePath)
{
	if (initializedRecognitionType != RecognitionType::Grammatic)
	{
		return "Error: This type of decoding is not initialized";
	}

	char *Cname = convertStringToChars(searchName);
	char *CfilePath = convertStringToChars(filePath);
	char *CcompleteFilePath = concat(applicationInstallFolderPath, CfilePath);

	int result = ps_set_lm_file(decoder, Cname, CcompleteFilePath);

	free(Cname);
	free(CfilePath);
	free(CcompleteFilePath);

	return (result == 0) ? 
		Platform::String::Concat(searchName, " Ngram search added") :
		Platform::String::Concat("fault adding Ngram search: ", searchName);
}

//// Add phones search (like .lm.bin files)
Platform::String^ SpeechRecognizer::AddPhonesSearch(Platform::String^ searchName, Platform::String^ filePath)
{
	if (initializedRecognitionType != RecognitionType::Phonemes)
	{
		return "Error: This type of decoding is not initialized";
	}

	char *Cname = convertStringToChars(searchName);
	char *CfilePath = convertStringToChars(filePath);
	char *CcompleteFilePath = concat(applicationInstallFolderPath, CfilePath);

	int result = ps_set_allphone_file(decoder, Cname, CcompleteFilePath);

	free(Cname);
	free(CfilePath);
	free(CcompleteFilePath);

	return (result == 0) ?
		Platform::String::Concat(searchName, " Phones search added") :
		Platform::String::Concat("fault adding Phones search: ", searchName);
}

#pragma endregion

#pragma region Control methods

//// Set search
Platform::String^ SpeechRecognizer::SetSearch(Platform::String^ searchName)
{
	char *CsearchName = convertStringToChars(searchName);

	int result = ps_set_search(decoder, CsearchName);

	free(CsearchName);

	if (result == 0)
	{
		isSearchSet = true;
		return Platform::String::Concat("Search set to: ", searchName);
	}
	else
	{
		return Platform::String::Concat("fault setting search to: ", searchName);
	}
}

Platform::String^ SpeechRecognizer::CleanPocketSphinx(void)
{
	if (initializedRecognitionType == RecognitionType::None || decoder == nullptr)
	{
		return "PocketSphinx is not initialized";
	}

	ps_free(decoder);
	initializedRecognitionType = RecognitionType::None;

	return "PocketSphinx resources cleaned";
}

#pragma endregion

#pragma region Continuous recognition

/// Start PocketSphinx processing (start utterance)
Platform::String^ SpeechRecognizer::StartProcessing(void)
{
	Platform::String^ message;
	if (!IsReadyForProcessing(message))
	{
		return message;
	}

	int result = ps_start_utt(decoder);

	isProcessing = (result == 0);

	return (result == 0) ?
		"PocketShinx ready for processing" :
		"Error starting PocketShinx processing";
}

/// Stop PocketSphinx processing (end utterance)
Platform::String^ SpeechRecognizer::StopProcessing(void)
{
	Platform::String^ message;
	if (!IsReadyForProcessing(message))
	{
		return message;
	}

	int result = -1;
	if (isProcessing)
	{
		result = ps_end_utt(decoder);
	}
	isInSpeech = false;
	lastHypothesis = "";
	isProcessing = false;

	return (result == 0) ?
		"PocketShinx stopped processing" :
		"Error stopping PocketShinx processing";
}

/// Stop en Start PocketSphinx processing
Platform::String^ SpeechRecognizer::RestartProcessing(void)
{
	Platform::String^ message;
	if (!IsReadyForProcessing(message))
	{
		return message;
	}

	int resultEnding = ps_end_utt(decoder);
	isProcessing = false;
	lastHypothesis = "";
	int resultStarting = ps_start_utt(decoder);
	isProcessing = (resultStarting == 0);

	return ((resultEnding + resultStarting) == 0 && (resultEnding - resultStarting) == 0) ?
		"PocketShinx restarted processing" :
		"Error restarting PocketShinx processing";
}

Platform::Boolean SpeechRecognizer::IsProcessing(void)
{
	return isProcessing;
}

int SpeechRecognizer::RegisterAudioBytes(const Platform::Array<uint8>^ audioBytes)
{
	Platform::String^ message;
	if (!IsReadyForProcessing(message))
	{
		return -1;
	}

	// Set Fields
	int32 audioLength, score;
	audioLength = audioBytes->Length / 2;
	int16 *audioBuffer = new int16[audioLength];

	char const *hypothesis;
	bool isCurrentInSpeech = false;

	// Convert ByteArray Array<uint8> to Int16[]
	for (size_t i = 0; i < audioBytes->Length; i += 2)
	{
		audioBuffer[i / 2] = audioBytes[i] + ((int16)audioBytes[i + 1] << 8);
	}

	// Proccess audio bytes -> return amount of processed bytes
	int result = ps_process_raw(decoder, audioBuffer, audioLength, FALSE, FALSE);

	// Get In speech indication (silent moments detection)
	isCurrentInSpeech = ps_get_in_speech(decoder);

	// Check if Hypothesis is found (recognized text)
	hypothesis = ps_get_hyp(decoder, &score);
	if (hypothesis != NULL)
	{
		// Report changed Hypothesis
		auto currentHypothesis = convertCharsToString(hypothesis);
		if (!currentHypothesis->Equals(lastHypothesis))
		{
			OnResultFound(currentHypothesis);
		}

		lastHypothesis = currentHypothesis;
	}

	// Report finalized Hypothesis (finalization of utterance)
	if (!isCurrentInSpeech && isInSpeech)
	{
		// Report Hypothesis
		OnResultFinalizedBySilence(lastHypothesis);

		// Restart procces cycle (to start new utterance)
		RestartProcessing();
	}

	isInSpeech = isCurrentInSpeech;
	delete[] audioBuffer;

	return result;
}

Platform::Boolean SpeechRecognizer::IsReadyForProcessing(Platform::String^& message)
{
	if (initializedRecognitionType == RecognitionType::None)
	{
		message = "PocketSphinx is not initialized";
		return false;
	}
	
	if (!isSearchSet)
	{
		message = "PocketSphinx has not loaded a search model or the search hasn't been set";
		return false;
	}

	return true;
}

#pragma region Utterance recognition

Platform::String^ SpeechRecognizer::GetHypothesisFromUtterance(const Platform::Array<uint8>^ audioBytes)
{
	// Check if system is not working with Continuous recognition
	if (isProcessing)
	{
		throw ref new Exception(1, "Can not use GetHypothesisFromUtterance while running Continuous recognition");
	}

	// Set Fields
	int32 audioLength, score;
	audioLength = audioBytes->Length / 2;
	int16 *audioBuffer = new int16[audioLength];
	
	// Convert ByteArray Array<uint8> to Int16[]
	for (size_t i = 0; i < audioBytes->Length; i += 2)
	{
		audioBuffer[i / 2] = audioBytes[i] + ((int16)audioBytes[i + 1] << 8);
	}

	// Proccess audio bytes
	int resultStart = ps_start_utt(decoder);
	int resultProcess = ps_process_raw(decoder, audioBuffer, audioLength, FALSE, TRUE);
	int resultEnd = ps_end_utt(decoder);

	// Get Hypothesis
	char const *hypothesis = ps_get_hyp(decoder, &score);
	auto hypothesisString = convertCharsToString(hypothesis);

	// Cleanup and return
	delete[] audioBuffer;
	return hypothesisString;
}

NbestHypotheses SpeechRecognizer::GetNbestFromUtterance
(const Platform::Array<uint8>^ audioBytes, int32 maximumNBestIterations)
{
	// Check if system is not working with Continuous recognition
	if (isProcessing)
	{
		throw ref new Exception(1, "Can not use GetHypothesisFromUtterance while running Continuous recognition");
	}

	// Set Fields
	NbestHypotheses nbestHypotheses = NbestHypotheses();
	char const *hypothesis;
	char* nbestHypothesesString = (char*)malloc(1); // Temp workaround till good struct
	strcpy(nbestHypothesesString, "|");  // Temp workaround till good struct

	ps_nbest_t *nbest;
	//auto vec = ref new Vector<Platform::String^>();
	int32 audioLength, score;
	audioLength = audioBytes->Length / 2;
	int16 *audioBuffer = new int16[audioLength];

	// Convert ByteArray Array<uint8> to Int16[]
	for (size_t i = 0; i < audioBytes->Length; i += 2)
	{
		audioBuffer[i / 2] = audioBytes[i] + ((int16)audioBytes[i + 1] << 8);
	}

	// Proccess audio bytes
	int resultStart = ps_start_utt(decoder);
	int resultProcess = ps_process_raw(decoder, audioBuffer, audioLength, FALSE, TRUE);
	int resultEnd = ps_end_utt(decoder);

	// Get Final Hypothesis
	hypothesis = ps_get_hyp(decoder, &score);
	nbestHypotheses.FinalHypothesis = convertCharsToString(hypothesis);
	nbestHypotheses.FinalHypothesisScore = score;
	
	// Get Nbest Hypotheses
	nbest = ps_nbest(decoder);
	for (size_t i = 0; i < maximumNBestIterations && nbest && (nbest = ps_nbest_next(nbest)); i++)
	{
		hypothesis = ps_nbest_hyp(nbest, &score);
		//vec->Append(convertCharsToString(hypothesis));

		if (hypothesis == NULL)
			continue;

		// Memories old string
		char* nbestHypothesesMem = (char*)malloc(strlen(nbestHypothesesString));
		strcpy(nbestHypothesesMem, nbestHypothesesString);

		// Free old string
		free(nbestHypothesesString);

		// Create new merge strings
		auto scoreChars =  convertStringToChars(score.ToString());

		nbestHypothesesString = (char*)malloc(strlen(nbestHypothesesMem) + strlen(hypothesis) + strlen(scoreChars) + 2);
		strcpy(nbestHypothesesString, nbestHypothesesMem);
		if (i != 0)
		{
			strcat(nbestHypothesesString, "|");
		}
		strcat(nbestHypothesesString, hypothesis);
		strcat(nbestHypothesesString, ":");
		strcat(nbestHypothesesString, scoreChars);

		// Cleanup memory
		free(scoreChars);
		free(nbestHypothesesMem);
	}
		
	nbestHypotheses.HypothesesAndScores = convertCharsToString(nbestHypothesesString);

	// Cleanup
	free(nbestHypothesesString);
	delete[] audioBuffer;
	if (nbest)
	{
		ps_nbest_free(nbest);
	}	

	return nbestHypotheses;
}

#pragma endregion

#pragma endregion

#pragma region Event Handlers

void SpeechRecognizer::OnResultFound(Platform::String^ result)
{
	this->resultFound(result);
}

void SpeechRecognizer::OnResultFinalizedBySilence(Platform::String^ finalResult)
{
	this->resultFinalizedBySilence(finalResult);
}

#pragma endregion




