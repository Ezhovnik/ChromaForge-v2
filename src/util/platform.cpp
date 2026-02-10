#include "platform.h"

#include <sstream>
#include <filesystem>
#include <iomanip>
#include <time.h>

#include "../typedefs.h"

#define SETTINGS_FILE "../build/settings.toml"
#define CONTROLS_FILE "../build/controls.json"

std::filesystem::path platform::get_settings_file() {
	return std::filesystem::path(SETTINGS_FILE);
}

std::filesystem::path platform::get_controls_file() {
	return std::filesystem::path(CONTROLS_FILE);
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
void platform::configure_encoding(){
}
#endif
