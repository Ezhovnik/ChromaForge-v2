#include "engine_paths.h"

#include <filesystem>
#include <sstream>
#include <algorithm>

#include "../typedefs.h"
#include "WorldFiles.h"
#include "../core_defs.h"

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

	std::filesystem::path filename = folder/std::filesystem::u8path("screenshot-" + datetimestr + "." + ext);
	uint index = 0;
	while (std::filesystem::exists(filename)) {
		filename = folder/std::filesystem::u8path("screenshot-" + datetimestr + "-" + std::to_string(index) + "." + ext);
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
    return folder/std::filesystem::u8path("ChromaForge.log");
}

std::filesystem::path EnginePaths::getWorldFolder() {
    return worldFolder;
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

void EnginePaths::setContentPacks(std::vector<ContentPack>* contentPacks) {
    this->contentPacks = contentPacks;
}

void EnginePaths::setWorldFolder(std::filesystem::path folder) {
    this->worldFolder = folder;
}

std::filesystem::path EnginePaths::resolve(std::string path) {
    size_t separator = path.find(':');
    if (separator == std::string::npos) return std::filesystem::u8path(path);

    std::string prefix = path.substr(0, separator);
    std::string filename = path.substr(separator + 1);

    if (prefix == "res" || prefix == BUILTIN_CONTENT_NAMESPACE) return resources/std::filesystem::u8path(filename);
    if (prefix == "user") return userfiles/std::filesystem::u8path(filename);
    if (prefix == "world") return worldFolder/std::filesystem::u8path(filename);

    if (contentPacks) {
        for (auto& pack : *contentPacks) {
            if (pack.id == prefix) return pack.folder/std::filesystem::u8path(filename);
        }
    }

    return std::filesystem::u8path("./" + filename);
}

std::vector<std::filesystem::path> EnginePaths::scanForWorlds() {
    std::vector<std::filesystem::path> folders;

    std::filesystem::path folder = getWorldsFolder();
    if (!std::filesystem::is_directory(folder)) return folders;
    
    for (auto entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_directory()) continue;

        std::filesystem::path worldFolder = entry.path();
        std::filesystem::path worldFile = worldFolder/std::filesystem::u8path(WorldFiles::WORLD_FILE);
        if (!std::filesystem::is_regular_file(worldFile)) continue;
        folders.push_back(worldFolder);
    }

    std::sort(folders.begin(), folders.end(), [](std::filesystem::path a, std::filesystem::path b) {
        a = a/std::filesystem::u8path(WorldFiles::WORLD_FILE);
        b = b/std::filesystem::u8path(WorldFiles::WORLD_FILE);
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });

    return folders;
}

ResPaths::ResPaths(std::filesystem::path mainRoot, std::vector<std::filesystem::path> roots) : mainRoot(mainRoot), roots(roots) {
}

std::filesystem::path ResPaths::find(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        std::filesystem::path file = root/std::filesystem::u8path(filename);
        if (std::filesystem::exists(file)) return file;
    }
    return mainRoot/std::filesystem::path(filename);
}

std::vector<std::filesystem::path> ResPaths::listdir(const std::string& folderName) const {
    std::vector<std::filesystem::path> entries;
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        std::filesystem::path folder = root/std::filesystem::u8path(folderName);
        if (!std::filesystem::is_directory(folder)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }

    {
        std::filesystem::path folder = mainRoot/std::filesystem::u8path(folderName);
        if (!std::filesystem::is_directory(folder)) return entries;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }
    return entries;
}

const std::filesystem::path& ResPaths::getMainRoot() const {
    return mainRoot;
}
