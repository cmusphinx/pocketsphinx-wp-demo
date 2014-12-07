/**
* Created by Toine de Boer, Enbyin (NL)
*
* intended as kick-start using PocketSphinx on Windows mobile platforms
*/

#include "SpeechRecognizer.h"

using namespace PocketSphinxRntComp;
using namespace Platform;

#include <pocketsphinx.h>
#include <sphinxbase/err.h>
#include <sphinxbase/jsgf.h>

#pragma region Info

// using pocketsphinx demo: http://cmusphinx.sourceforge.net/wiki/tutorialpocketsphinx
// api pocketsphinx: http://cmusphinx.sourceforge.net/doc/pocketsphinx/

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

#pragma endregion

#pragma region Private Fields

bool isInitialized = false;
bool hasSearchBeenSet = false;

bool isPreviousInSpeech = false;

bool isProcessing = false;

Platform::String^ previousHyp = "";

ps_decoder_t *ps;
int score;
const char *hyp;

char installedFolderPath[1024];
char localStorageFolder[1024];
wchar_t whyp[1024];

#pragma endregion

#pragma region Constructor

SpeechRecognizer::SpeechRecognizer()
{
}

#pragma endregion

#pragma region Static Helpers

static char* concat(char *s1, char *s2)
{
	char *result = (char *)malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

static char* convertStringToChars(Platform::String^ input)
{
	const wchar_t *W = input->Data();

	int Size = wcslen(W);
	char *CString = new char[Size + 1];
	CString[Size] = 0;
	for (int y = 0; y < Size; y++)
	{
		CString[y] = (char)W[y];
	}

	return CString;
}

String^ convertCharsToString(const char* chars)
{
	static wchar_t buffer[128];
	mbstowcs(buffer, chars, 128);
	return ref new Platform::String(buffer);
}

#pragma endregion

#pragma region Initialize and Load Methods

//// Initialize decoder with default models
/// hmmFolderPath	== the acoustic model folder (content folder: http://cmusphinx.sourceforge.net/wiki/tutorialam#using_the_model)
/// dictFilePath	== the dictionary file
Platform::String^ SpeechRecognizer::Initialize(Platform::String^ hmmFolderPath, Platform::String^ dictFilePath)
{
	if (isInitialized)
	{
		return "PocketSphinx is already initialized";
	}

	cmd_ln_t *config;

	// Local Storage Path
	wcstombs(localStorageFolder, Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data(), 1024);
	// Installed Folder path
	wcstombs(installedFolderPath, Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data(), 1024);

	//Error file path
	char *logPath = concat(localStorageFolder, "\\errors.log");
	//err_set_logfile(logPath);

	char *hmmPath = concat(installedFolderPath, convertStringToChars(hmmFolderPath));
	char *dictPath = concat(installedFolderPath, convertStringToChars(dictFilePath));

	config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", hmmPath,
		"-dict", dictPath,
		"-mmap", "no",
		"-logfn", logPath,
		"-kws_threshold", "1e-40",
		NULL);

	if (config == NULL)
		return "No config";

	ps = ps_init(config);
	if (ps == NULL)
		return "No decoder";

	isInitialized = true;
	return "PocketSphinx initialization done";
}

//// Add keyword-activation search
Platform::String^ SpeechRecognizer::AddKeyphraseSearch(Platform::String^ name, Platform::String^ keyphrase)
{
	char *Cname = convertStringToChars(name);
	char *Ckeyphrase = convertStringToChars(keyphrase);

	int result = ps_set_keyphrase(ps, Cname, Ckeyphrase);

	return (result == 0) ?
		Platform::String::Concat(name, " keyphrase search added") :
		Platform::String::Concat("fault adding keyphrase search: ", name);
}

//// Add grammar-based searches (like .jsgf or .gram files)
Platform::String^ SpeechRecognizer::AddGrammarSearch(Platform::String^ name, Platform::String^ filePath)
{
	char *Cname = convertStringToChars(name);
	char *CfilePath = convertStringToChars(filePath);
	char *CcompleteFilePath = concat(installedFolderPath, CfilePath);

	//fsg_model_t *pNewFSGModel = jsgf_read_file(CcompleteFilePath, ps_get_logmath(ps), 6.5);
	//int result = ps_set_fsg(ps, Cname, pNewFSGModel);
	int result = ps_set_jsgf_file(ps, Cname, CcompleteFilePath);

	// 0 == OK, 1 == fault
	return (result == 0)? 
		Platform::String::Concat(name, " grammar search added"):
		Platform::String::Concat("fault adding grammar search: ", name);
}

//// Add language model search (like *.dmp files)
Platform::String^ SpeechRecognizer::AddNgramSearch(Platform::String^ name, Platform::String^ filePath)
{
	char *Cname = convertStringToChars(name);
	char *CfilePath = convertStringToChars(filePath);
	char *CcompleteFilePath = concat(installedFolderPath, CfilePath);

	int result = ps_set_lm_file(ps, Cname, CcompleteFilePath);

	return (result == 0) ? 
		Platform::String::Concat(name, " Ngram search added"):
		Platform::String::Concat("fault adding Ngram search: ", name);
}

#pragma endregion

#pragma region Operation methods

/// Start PocketSphinx processing (utt)
Platform::String^ SpeechRecognizer::StartProcessing(void)
{
	Platform::String^ message;
	if (!IsReadyForProcessing(message))
	{
		return message;
	}

	auto result = ps_start_utt(ps, NULL);

	isProcessing = (result == 0);

	return (result == 0) ?
		"PocketShinx ready for processing" :
		"Error starting PocketShinx processing";
}

/// Stop PocketSphinx processing (utt)
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
		result = ps_end_utt(ps);
	}
	isPreviousInSpeech = false;
	previousHyp = "";
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

	auto resultEnding = ps_end_utt(ps);
	isProcessing = false;
	previousHyp = "";
	auto resultStarting = ps_start_utt(ps, NULL);
	isProcessing = (resultStarting == 0);

	return ((resultEnding + resultStarting) == 0 && (resultEnding - resultStarting) == 0) ?
		"PocketShinx restarted processing" :
		"Error restarting PocketShinx processing";
}

Platform::Boolean SpeechRecognizer::IsProcessing(void)
{
	return isProcessing;
}

/// Register Audio Bytes -> 
/// 
int SpeechRecognizer::RegisterAudioBytes(const Platform::Array<uint8>^ audioBytes)
{
	Platform::String^ message;
	if (!IsReadyForProcessing(message))
	{
		return -1;
	}

	// litle info @ http://blog.csdn.net/zouxy09/article/details/7978108

	int const buffSize = 16384;
	int16 audioBuffer[buffSize];
	int32 k, ts, rem, score;
	char const *hyp;
	char const *uttid;
	char word[256];
	bool isCurrentInSpeech = false;


	// Length for Int16[]
	k = audioBytes->Length / 2;
	if (k > buffSize)
	{
		auto errorMessage = Platform::String::Concat("audioBuffer out of bound at RegisterAudioBytes() with bytecount: ", k.ToString());
		throw ref new OutOfBoundsException(errorMessage);
	}

	// Convert ByteArray Array<uint8> to Int16[]
	for (size_t i = 0; i < audioBytes->Length; i += 2)
	{
		audioBuffer[i / 2] = audioBytes[i] + ((int16)audioBytes[i + 1] << 8);
	}

	// Proccess bytes
	int result = ps_process_raw(ps, audioBuffer, k, FALSE, FALSE);

	// Detect speech
	isCurrentInSpeech = ps_get_in_speech(ps);

	// Get Hyp / Detection
	hyp = ps_get_hyp(ps, &score, &uttid);
	//hyp = ps_get_hyp_final(ps, &score);
	if (hyp != NULL)
	{
		// Result found
		//auto hypString = convertCharsToString(hyp);
		auto newHyp = convertCharsToString(hyp);

		//if (hypString->Equals("oh mighty computer"))
		//{
		//	auto resultString = Platform::String::Concat("Recognized: ", hypString);
		//	OnResultFound(resultString);
		//}

		if (!newHyp->Equals(previousHyp))
		{
			OnResultFound(newHyp);
		}

		previousHyp = newHyp;
	}

	// Get Hyp when speech ended then restart process cycle
	if (!isCurrentInSpeech && isPreviousInSpeech)
	{
		OnResultFinalizedBySilence(previousHyp);

		// Restart procces cycle (to get clean Hyp)
		RestartProcessing();
	}

	isPreviousInSpeech = isCurrentInSpeech;

	return result;
}


//// Set search
Platform::String^ SpeechRecognizer::SetSearch(Platform::String^ name)
{
	char *Cname = convertStringToChars(name);

	int result = ps_set_search(ps, Cname);

	if (result == 0)
	{
		hasSearchBeenSet = true;
		return Platform::String::Concat("Search set to: ", name);
	}
	else
	{
		return Platform::String::Concat("fault setting search to: ", name);
	}
}

//// Cleanup PocketSphinx resources
Platform::String^ SpeechRecognizer::CleanPocketSphinx(void)
{
	if (!isInitialized || ps == nullptr)
	{
		return "PocketSphinx is not initialized";
	}

	ps_free(ps);
	isInitialized = false;

	return "PocketSphinx resources cleaned";
}

Platform::Boolean SpeechRecognizer::IsReadyForProcessing(Platform::String^& message)
{
	if (!isInitialized)
	{
		message = "PocketSphinx is not initialized";
		return false;
	}
	
	if (!hasSearchBeenSet)
	{
		message = "PocketSphinx has not loaded a search model or the search hasn't been set";
		return false;
	}

	return true;
}

#pragma region Test Methods

//// Test with recorded recording (go 10 years....
Platform::String^ SpeechRecognizer::TestPocketSphinx(void)
{
	FILE *fh;
	fh = fopen(concat(installedFolderPath, "\\Assets\\models\\goforward.raw"), "rb");
	if (fh == NULL) {
		return "No file";
	}

	ps_decode_raw(ps, fh, "goforward", -1);
	hyp = ps_get_hyp(ps, &score, NULL);

	mbstowcs(whyp, hyp, 1024);

	fclose(fh);
	return ref new String(whyp);
}

#pragma endregion

#pragma endregion

#pragma region Private Methods

#pragma region Event Handlers

void SpeechRecognizer::OnResultFinalizedBySilence(Platform::String^ finalResult)
{
	this->resultFinalizedBySilence(finalResult);
}

void SpeechRecognizer::OnResultFound(Platform::String^ result)
{
	this->resultFound(result);

	// to UI Thread: (can also be done in UI Project)
	//auto window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
	//auto m_dispatcher = window->Dispatcher;
	//// Since this code is probably running on a worker  
	//// thread, and we are passing the data back to the  
	//// UI thread, we have to use a CoreDispatcher object.
	//m_dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
	//	ref new  Windows::UI::Core::DispatchedHandler([this, result]()
	//{
	//	this->resultFoundEvent(result);

	//}, Platform::CallbackContext::Any));
}

#pragma endregion

#pragma endregion



