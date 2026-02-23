#include "platform.h"

#include <sstream>
#include <filesystem>
#include <iomanip>
#include <time.h>
#include <algorithm>

#include "../typedefs.h"
#include "../logger/Logger.h"

namespace platform {
    const std::string DEFAULT_LOCALE = "en_US";
}

#ifdef _WIN32
#include <Windows.h>
#include <cstdio>

void platform::configure_encoding() {
	// set utf-8 encoding to console output
	SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}
#else
void platform::configure_encoding() {
}
#endif

#ifdef _WIN32
#include <Windows.h>

std::string platform::detect_locale() {
	wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH) > 0) {
        char buffer[256];
        WideCharToMultiByte(CP_UTF8, 0, localeName, -1, buffer, sizeof(buffer), nullptr, nullptr);
        std::string result(buffer);
		std::replace(result.begin(), result.end(), '-', '_');
        LOG_DEBUG("Detected environment language local: {}", result);
		return result;
    } else {
        LOG_DEBUG("Detected environment language local: {}", platform::DEFAULT_LOCALE);
        return platform::DEFAULT_LOCALE;
    }
}
#else
#include <cstdlib>
#include <locale.h>

std::string platform::detect_locale() {
	const char* lang = getenv("LC_ALL");
    if (!lang || *lang == '\0') lang = getenv("LANG");
    if (!lang || *lang == '\0') lang = platform::DEFAULT_LOCALE;

    std::string result(lang);
    size_t dotPos = result.find('.');
    if (dotPos != std::string::npos) result = result.substr(0, dotPos);
    LOG_DEBUG("Detected environment language local: {}", result);
    return result;
}
#endif
