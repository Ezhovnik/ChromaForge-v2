#include "engine_files.h"

#include <string>
#include <filesystem>

#include "../typedefs.h"

std::string engine_fs::get_screenshot_file(std::string ext) {
	std::string folder = SCREENSHOTS_FOLDER;
	if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%d-%m-%Y_%H-%M-%S";

	std::stringstream ss;
	ss << std::put_time(&tm, format);
	std::string datetimestr = ss.str();

	std::string filename = folder + "/screenshot-" + datetimestr + "." + ext;
	uint index = 0;
	while (std::filesystem::exists(filename)) {
		filename = folder + "screenshot-" + datetimestr + "-" + std::to_string(index) + "." + ext;
		index++;
	}
	return filename;
}

std::string engine_fs::get_saves_folder() {
    std::string folder = SAVES_FOLDER;
    if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);
    return folder;
}
