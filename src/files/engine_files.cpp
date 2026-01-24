#include "engine_files.h"

#include <string>
#include <filesystem>

#include "../typedefs.h"

#define SCREENSHOTS_FOLDER "../build/screenshots"
#define SAVES_FOLDER "../build/saves"
#define LOGS_FOLDER "../build/logs"
#define ICON_FOLDER "../res/icon"

std::filesystem::path engine_fs::get_screenshot_file(std::string ext) {
	std::filesystem::path folder = SCREENSHOTS_FOLDER;
	if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%d-%m-%Y_%H-%M-%S";

	std::stringstream ss;
	ss << std::put_time(&tm, format);
	std::string datetimestr = ss.str();

	std::filesystem::path filename = folder/std::filesystem::path("/screenshot-" + datetimestr + "." + ext);
	uint index = 0;
	while (std::filesystem::exists(filename)) {
		filename = folder/std::filesystem::path("screenshot-" + datetimestr + "-" + std::to_string(index) + "." + ext);
		index++;
	}
	return filename;
}

std::filesystem::path engine_fs::get_saves_folder() {
    std::filesystem::path folder = SAVES_FOLDER;
    if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);
    return folder;
}

std::filesystem::path engine_fs::get_logs_file() {
    std::filesystem::path folder = LOGS_FOLDER;
    if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);
    return folder/std::filesystem::path("ChromaForge.log");
}

std::filesystem::path engine_fs::get_icon_file(int index) {
    std::filesystem::path folder = ICON_FOLDER;
    return folder/std::filesystem::path("icon" + std::to_string(index) + ".png");
}
