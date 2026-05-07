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

static inline auto SCREENSHOTS_FOLDER = std::filesystem::u8path("screenshots");
static inline auto CONTENT_FOLDER = std::filesystem::u8path("content");
static inline auto LOGS_FOLDER = std::filesystem::u8path("logs");
static inline auto WORLDS_FOLDER = std::filesystem::u8path("saves");
static inline auto CONFIG_FOLDER = std::filesystem::u8path("config");
static inline auto CONTROLS_FILE = std::filesystem::u8path("controls.toml");
static inline auto SETTINGS_FILE = std::filesystem::u8path("settings.toml");

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
    auto contentFolder = userFilesFolder/CONTENT_FOLDER;
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

std::filesystem::path EnginePaths::getControlsFile() const {
    return userFilesFolder/CONTROLS_FILE;
}

std::filesystem::path EnginePaths::getSettingsFile() const {
    return userFilesFolder/SETTINGS_FILE;
}

std::filesystem::path EnginePaths::getNewScreenshotFile(const std::string& ext) {
	auto folder = userFilesFolder/SCREENSHOTS_FOLDER;
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

std::filesystem::path EnginePaths::getWorldsFolder() const {
    return userFilesFolder/WORLDS_FOLDER;
}

std::filesystem::path EnginePaths::getConfigFolder() const {
    return userFilesFolder/CONFIG_FOLDER;
}

std::filesystem::path EnginePaths::getLogsFile() const {
    auto folder = userFilesFolder/LOGS_FOLDER;
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

std::tuple<std::string, std::string> EnginePaths::parsePath(std::string_view path) {
    size_t separator = path.find(':');
    if (separator == std::string::npos) {
        return {"", std::string(path)};
    }
    auto prefix = std::string(path.substr(0, separator));
    auto filename = std::string(path.substr(separator + 1));
    return {prefix, filename};
}

std::filesystem::path EnginePaths::resolve(const std::string& path, bool throwErr) {
    auto [prefix, filename] = EnginePaths::parsePath(path);
    if (prefix.empty()) {
        LOG_ERROR("No entry point specified");
        throw files_access_error("No entry point specified");
    }

    filename = toCanonic(std::filesystem::u8path(filename)).u8string();

    if (prefix == "res" || prefix == BUILTIN_CONTENT_NAMESPACE) return resourcesFolder/std::filesystem::u8path(filename);
    if (prefix == "user") return userFilesFolder/std::filesystem::u8path(filename);
    if (prefix == "config") return getConfigFolder()/std::filesystem::u8path(filename);
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
    return entries;
}

dv::value ResPaths::readCombinedList(const std::string& filename) {
    dv::value list = dv::list();
    for (const auto& root : roots) {
        auto path = root.path / std::filesystem::u8path(filename);
        if (!std::filesystem::exists(path)) continue;

        try {
            auto value = files::read_object(path);
            if (!value.isList()) {
                LOG_WARN("Reading combined list {}: {} is not a list (skipped)", root.name, filename);
                continue;
            }
            for (const auto& elem : value) {
                list.add(elem);
            }
        } catch (const std::runtime_error& err) {
            LOG_WARN("Reading combined list {}: {}: {}", root.name, filename, err.what());
        }
    }
    return list;
}

const std::filesystem::path& ResPaths::getMainRoot() const {
    return mainRoot;
}
