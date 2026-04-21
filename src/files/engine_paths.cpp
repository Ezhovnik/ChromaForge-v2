#include <files/engine_paths.h>

#include <filesystem>
#include <sstream>
#include <algorithm>
#include <stack>
#include <utility>

#include <typedefs.h>
#include <files/WorldFiles.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <util/stringutil.h>

enum F_F_NAME{
    SCREENSHOTS_FOLDER,
    CONTENT_FOLDER,
    LOGS_FOLDER,
    CONTROLS_FILE,
    SETTINGS_FILE,

    COUNT
};

static std::array<std::string, F_F_NAME::COUNT> f_f_names{
    "screenshots",
    "content",
    "logs",
    "controls.toml",
    "settings.toml"
};


static std::filesystem::path toCanonic(std::filesystem::path path) {
    std::stack<std::string> parts;
    path = path.lexically_normal();
    do {
        parts.push(path.filename().u8string());
        path = path.parent_path();
    } while (!path.empty());
    path = std::filesystem::u8path("");
    while (!parts.empty()) {
        const std::string part = parts.top();
        parts.pop();
        if (part == ".") continue;
        if (part == "..") {
            LOG_ERROR("Entry point reached");
            throw files_access_error("Entry point reached");
        }
        path = path/std::filesystem::path(part);
    }
    return path;
}

void EnginePaths::prepare() {
    auto contentFolder = userFilesFolder/std::filesystem::path(f_f_names[CONTENT_FOLDER]);
    if (!std::filesystem::is_directory(contentFolder)) {
        std::filesystem::create_directories(contentFolder);
    }
}

std::filesystem::path EnginePaths::getUserFilesFolder() const {
	return userFilesFolder;
}

std::filesystem::path EnginePaths::getResourcesFolder() const {
	return resourcesFolder;
}

std::filesystem::path EnginePaths::getControlsFile() {
    return userFilesFolder/std::filesystem::path(f_f_names[CONTROLS_FILE]);
}

std::filesystem::path EnginePaths::getSettingsFile() {
    return userFilesFolder/std::filesystem::path(f_f_names[SETTINGS_FILE]);
}

std::filesystem::path EnginePaths::getNewScreenshotFile(const std::string& ext) {
	auto folder = userFilesFolder/std::filesystem::path(f_f_names[SCREENSHOTS_FOLDER]);
	if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%Y-%m-%d_%H-%M-%S";
	std::stringstream ss;
	ss << std::put_time(&tm, format);
	std::string datetimestr = ss.str();

	auto filename = folder/std::filesystem::u8path("screenshot-" + datetimestr + "." + ext);
	uint index = 0;
	while (std::filesystem::exists(filename)) {
		filename = folder/std::filesystem::u8path("screenshot-" + datetimestr + "-" + std::to_string(index) + "." + ext);
		index++;
	}
	return filename;
}

std::filesystem::path EnginePaths::getWorldsFolder() {
    return userFilesFolder/std::filesystem::path("saves");
}

std::filesystem::path EnginePaths::getLogsFile() {
    auto folder = std::filesystem::path(f_f_names[LOGS_FOLDER]);
    if (!std::filesystem::is_directory(folder)) std::filesystem::create_directory(folder);
    return folder/std::filesystem::u8path("ChromaForge.log");
}

std::filesystem::path EnginePaths::getCurrentWorldFolder() {
    return currentWorldFolder;
}

std::filesystem::path EnginePaths::getWorldFolderByName(const std::string& name) {
    return getWorldsFolder()/std::filesystem::path(name);
}

void EnginePaths::setUserFilesFolder(std::filesystem::path folder) {
    this->userFilesFolder = std::move(folder);
}

void EnginePaths::setResourcesFolder(std::filesystem::path folder) {
    this->resourcesFolder = std::move(folder);
}

void EnginePaths::setContentPacks(std::vector<ContentPack>* contentPacks) {
    this->contentPacks = contentPacks;
}

void EnginePaths::setCurrentWorldFolder(std::filesystem::path folder) {
    this->currentWorldFolder = std::move(folder);
}

std::filesystem::path EnginePaths::resolve(const std::string& path, bool throwErr) {
    size_t separator = path.find(':');
    if (separator == std::string::npos) {
        LOG_ERROR("No entry point specified");
        throw files_access_error("No entry point specified");
    }

    std::string prefix = path.substr(0, separator);
    std::string filename = path.substr(separator + 1);
    filename = toCanonic(std::filesystem::u8path(filename)).u8string();

    if (prefix == "res" || prefix == BUILTIN_CONTENT_NAMESPACE) return resourcesFolder/std::filesystem::u8path(filename);
    if (prefix == "user") return userFilesFolder/std::filesystem::u8path(filename);
    if (prefix == "world") return currentWorldFolder/std::filesystem::u8path(filename);

    if (contentPacks) {
        for (auto& pack : *contentPacks) {
            if (pack.id == prefix) return pack.folder/std::filesystem::u8path(filename);
        }
    }

    LOG_ERROR("Unknown entry point '{}'", prefix);
    if (throwErr) throw files_access_error("Unknown entry point '" + prefix + "'");
    return std::filesystem::path(filename);
}

std::vector<std::filesystem::path> EnginePaths::scanForWorlds() {
    std::vector<std::filesystem::path> folders;

    auto folder = getWorldsFolder();
    if (!std::filesystem::is_directory(folder)) return folders;

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_directory()) continue;

        const std::filesystem::path& worldFolder = entry.path();
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

ResPaths::ResPaths(
    std::filesystem::path mainRoot, 
    std::vector<PathsRoot> roots
) : mainRoot(std::move(mainRoot)), roots(std::move(roots)) {}

std::filesystem::path ResPaths::find(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        auto file = root.path/std::filesystem::u8path(filename);
        if (std::filesystem::exists(file)) return file;
    }
    return mainRoot/std::filesystem::path(filename);
}

std::string ResPaths::findRaw(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        if (std::filesystem::exists(root.path/std::filesystem::path(filename))) {
            return root.name + ":" + filename;
        }
    }
    auto resDir = mainRoot;
    if (std::filesystem::exists(resDir/std::filesystem::path(filename))) {
        return BUILTIN_CONTENT_NAMESPACE + ":" + filename;
    }
    LOG_ERROR("Could not to find file '{}'", filename);
    throw std::runtime_error("Could not to find file " + util::quote(filename));
}

std::vector<std::string> ResPaths::listdirRaw(const std::string& folderName) const {
    std::vector<std::string> entries;
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        auto folder = root.path/std::filesystem::u8path(folderName);
        if (!std::filesystem::is_directory(folder)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            auto name = entry.path().filename().u8string();
            entries.emplace_back(root.name + ":" + folderName + "/" + name);
        }
    }
    {
        auto folder = mainRoot/std::filesystem::u8path(folderName);
        if (!std::filesystem::is_directory(folder)) return entries;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            auto name = entry.path().filename().u8string();
            entries.emplace_back(BUILTIN_CONTENT_NAMESPACE + ":" + folderName + "/" + name);
        }
    }
    return entries;
}

std::vector<std::filesystem::path> ResPaths::listdir(const std::string& folderName) const {
    std::vector<std::filesystem::path> entries;
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        auto folder = root.path/std::filesystem::u8path(folderName);
        if (!std::filesystem::is_directory(folder)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }

    {
        auto folder = mainRoot/std::filesystem::u8path(folderName);
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
