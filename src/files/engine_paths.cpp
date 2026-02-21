#include "engine_paths.h"

#include <filesystem>
#include <sstream>
#include "../typedefs.h"

#define BUILD_FOLDER "../build"
#define SCREENSHOTS_FOLDER BUILD_FOLDER"/screenshots"
#define LOGS_FOLDER BUILD_FOLDER"/logs"
#define SAVES_FOLDER BUILD_FOLDER"/saves"

std::filesystem::path EnginePaths::getUserfiles() const {
	return userfiles;
}

std::filesystem::path EnginePaths::getResources() const {
	return resources;
}

std::filesystem::path EnginePaths::getScreenshotFile(std::string ext) {
	std::filesystem::path folder = SCREENSHOTS_FOLDER;
	if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%Y-%m-%d_%H-%M-%S";
	std::stringstream ss;
	ss << std::put_time(&tm, format);
	std::string datetimestr = ss.str();

	std::filesystem::path filename = folder/std::filesystem::path("screenshot-" + datetimestr + "." + ext);
	uint index = 0;
	while (std::filesystem::exists(filename)) {
		filename = folder/std::filesystem::path("screenshot-" + datetimestr + "-" + std::to_string(index) + "." + ext);
		index++;
	}
	return filename;
}

std::filesystem::path EnginePaths::getWorldsFolder() {
    std::filesystem::path folder = std::filesystem::path(SAVES_FOLDER);
    if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);
    return folder;
}

std::filesystem::path EnginePaths::getLogsFile() {
    std::filesystem::path folder = std::filesystem::path(LOGS_FOLDER);
    if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);
    return folder/std::filesystem::path("ChromaForge.log");
}

bool EnginePaths::isWorldNameUsed(std::string name) {
	return std::filesystem::exists(EnginePaths::getWorldsFolder()/std::filesystem::u8path(name));
}

void EnginePaths::setUserfiles(std::filesystem::path folder) {
	this->userfiles = folder;
}

void EnginePaths::setResources(std::filesystem::path folder) {
	this->resources = folder;
}

ResPaths::ResPaths(std::filesystem::path mainRoot, std::vector<std::filesystem::path> roots) : mainRoot(mainRoot), roots(roots) {
}

std::filesystem::path ResPaths::find(const std::string& filename) const {
    for (auto& root : roots) {
        std::filesystem::path file = root/std::filesystem::path(filename);
        if (std::filesystem::exists(file)) return file;
    }
    return mainRoot/std::filesystem::path(filename);
}

std::vector<std::filesystem::path> ResPaths::listdir(const std::string& folderName) const {
    std::vector<std::filesystem::path> entries;
    for (auto& root : roots) {
        std::filesystem::path folder = root/std::filesystem::path(folderName);
        if (!std::filesystem::is_directory(folder)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }

    {
        std::filesystem::path folder = mainRoot/std::filesystem::path(folderName);
        if (!std::filesystem::is_directory(folder)) return entries;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }
    return entries;
}
