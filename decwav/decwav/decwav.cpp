#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>

#include "TTSAPI.H"

#define MMSUCCEEDED(x) ((x) == MMSYSERR_NOERROR)
#define MMFAILED(x) (!MMSUCCEEDED(x))
#define ABORT(fn, code) { \
		std::cout << "Error at " << __FILE__ << ":" << __LINE__ << " after call to " << fn << ": " << code << std::endl << std::flush; \
		exit(1); \
	}

static void
tts_init(LPTTS_HANDLE_T *handle)
{
	MMRESULT res = TextToSpeechStartup(
		/* HWND */ nullptr,
		handle,
		WAVE_MAPPER,
		DO_NOT_USE_AUDIO_DEVICE
	);
	if (MMFAILED(res)) {
		ABORT("TextToSpeechStartup", res);
	}
}

static void
tts_speak(LPTTS_HANDLE_T handle, const std::string& filename, const std::string& text)
{
	MMRESULT res;
	if (MMFAILED(res = TextToSpeechOpenWaveOutFile(handle, (char *) filename.c_str(), WAVE_FORMAT_1M16))) {
		ABORT("TextToSpeechOpenWaveOutFile", res);
	}
	// Turn phoneme modifiers on by default for compatibility
	if (MMFAILED(res = TextToSpeechSpeak(handle, "[:phone on]", TTS_FORCE))) {
		ABORT("TextToSpeechSpeak", res);
	}
	if (MMFAILED(res = TextToSpeechSpeak(handle, (char *) text.c_str(), TTS_FORCE))) {
		ABORT("TextToSpeechSpeak", res);
	}
	if (MMFAILED(res = TextToSpeechSync(handle))) {
		ABORT("TextToSpeechSync", res);
	}
	if (MMFAILED(res = TextToSpeechCloseWaveOutFile(handle))) {
		ABORT("TextToSpeechCloseWaveOutFile", res);
	}
}

static void
tts_close(LPTTS_HANDLE_T *handle)
{
	MMRESULT res = TextToSpeechShutdown(*handle);
	if (MMFAILED(res)) {
		ABORT("TextToSpeechShutdown", res);
	}

	*handle = nullptr;
}

std::string readFile(const std::string& fileName) {
	std::ifstream file(fileName);
	if (file.is_open()) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();
		return buffer.str();
	}
	else {
		std::cerr << "Unable to open file: " << fileName << std::endl;
		return "";
	}
}

int
main(void)
{
	LPTTS_HANDLE_T handle = nullptr;

	while (true) {
		std::string filename;
		std::string text;

		tts_init(&handle);
		std::cout << "Ready" << std::endl;

		std::getline(std::cin, filename);
		//std::getline(std::cin, text);
		
		if (std::cin.fail() || std::cin.eof())
			break;

		text = readFile("input/" + filename + ".txt");

		tts_speak(handle, "output/" + filename + ".wav", text);
		std::cout << "Success" << std::endl;

		tts_close(&handle);
	}

	return 0;
}