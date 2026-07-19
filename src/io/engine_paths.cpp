#include <io/engine_paths.h>

#include <sstream>
#include <algorithm>
#include <stack>
#include <utility>
#include <chrono>

#include <typedefs.h>
#include <world/files/WorldFiles.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <util/stringutil.h>
#include <io/devices/StdfsDevice.h>
#include <math/rand.h>
#include <io/devices/ZipFileDevice.h>

template<int n>
static std::string generate_random_base64() {
    auto now = std::chrono::high_resolution_clock::now();
    auto seed = now.time_since_epoch().count();

    util::PseudoRandom random(seed); // FIXME: Replace with safe random
    ubyte bytes[n];
    random.rand(bytes, n);
    return util::base64_urlsafe_encode(bytes, n);
}

static inline io::path SCREENSHOTS_FOLDER = "user:screenshots";
static inline io::path CONTENT_FOLDER = "user:content";
static inline io::path WORLDS_FOLDER = "user:saves";

void EnginePaths::prepare() {
    io::set_device("res", std::make_shared<io::StdfsDevice>(resourcesFolder, false));
    io::set_device("user", std::make_shared<io::StdfsDevice>(userFilesFolder));

    if (!io::is_directory("res:")) {
        LOG_ERROR("{} is not a directory", resourcesFolder.u8string());
        throw std::runtime_error(
            resourcesFolder.u8string() + " is not a directory"
        );
    }

    LOG_INFO("Resources folder: {}", std::filesystem::canonical(resourcesFolder).u8string());
    LOG_INFO("User files folder: {}", std::filesystem::canonical(userFilesFolder).u8string());
    LOG_INFO("Project folder: {}", std::filesystem::canonical(projectFolder).u8string());

    if (!io::is_directory(CONTENT_FOLDER)) {
        io::create_directories(CONTENT_FOLDER);
    }

    io::create_subdevice("builtin", "res", "");
    io::create_subdevice("export", "user", "export");
    io::create_subdevice("config", "user", "config");
}

const std::filesystem::path& EnginePaths::getUserFilesFolder() const {
	return userFilesFolder;
}

const std::filesystem::path& EnginePaths::getResourcesFolder() const {
	return resourcesFolder;
}

io::path EnginePaths::getNewScreenshotFile(const std::string& ext) {
	auto folder = SCREENSHOTS_FOLDER;
	if (io::is_directory(folder)) io::create_directories(folder);

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%Y-%m-%d_%H-%M-%S";
	std::stringstream ss;
	ss << std::put_time(&tm, format);
	std::string datetimestr = ss.str();

	auto file = folder / ("screenshot-" + datetimestr + "." + ext);
	uint index = 0;
	while (io::exists(file)) {
		file = folder / ("screenshot-" + datetimestr + "-" + std::to_string(index) + "." + ext);
		index++;
	}
	return file;
}

io::path EnginePaths::getWorldsFolder() const {
    return WORLDS_FOLDER;
}

io::path EnginePaths::getCurrentWorldFolder() {
    return currentWorldFolder;
}

io::path EnginePaths::getWorldFolderByName(const std::string& name) {
    return getWorldsFolder() / name;
}

void EnginePaths::setUserFilesFolder(std::filesystem::path folder) {
    this->userFilesFolder = std::move(folder);
}

void EnginePaths::setResourcesFolder(std::filesystem::path folder) {
    this->resourcesFolder = std::move(folder);
}

void EnginePaths::cleanup() {
    for (const auto& [id, _] : entryPoints) {
        io::remove_device(id);
    }
    for (const auto& [_, entryPoint] : writeables) {
        io::remove_device(entryPoint);
    }
    for (const auto& entryPoint : mounted) {
        io::remove_device(entryPoint);
    }

    entryPoints.clear();
    writeables.clear();
}

void EnginePaths::setEntryPoints(std::vector<PathsRoot> entryPoints) {
    cleanup();

    for (const auto& point : entryPoints) {
        auto parent = point.path.entryPoint();
        io::create_subdevice(point.name, parent, point.path);
    }
    this->entryPoints = std::move(entryPoints);
}

std::string EnginePaths::mount(const io::path& file) {
    if (file.extension() == ".zip") {
        auto stream = io::read(file);
        auto device = std::make_unique<io::ZipFileDevice>(
            std::move(stream), [file]() { return io::read(file); }
        );
        std::string name;
        do {
            name = std::string("M.") + generate_random_base64<6>();
        } while (std::find(mounted.begin(), mounted.end(), name) != mounted.end());

        io::set_device(name, std::move(device));
        mounted.push_back(name);
        return name;
    }
    LOG_ERROR("Unable to mount {}", file.string());
    throw std::runtime_error("Unable to mount " + file.string());
}

void EnginePaths::unmount(const std::string& name) {
    const auto& found = std::find(mounted.begin(), mounted.end(), name);
    if (found == mounted.end()) {
        LOG_ERROR("{} is not mounted", name);
        throw std::runtime_error(name + " is not mounted");
    }
    io::remove_device(name);
    mounted.erase(found);
}

std::string EnginePaths::createWriteableDevice(const std::string& name) {
    const auto& found = writeables.find(name);
    if (found != writeables.end()) return found->second;

    io::path folder;
    for (const auto& point : entryPoints) {
        if (point.name == name) {
            folder = point.path;
            break;
        }
    }
    if (name == BUILTIN_CONTENT_NAMESPACE) folder = "res:";
    if (folder.emptyOrInvalid()) {
        LOG_ERROR("Pack not found");
        throw std::runtime_error("Pack not found");
    }

    auto entryPoint = std::string("W.") + generate_random_base64<6>();

    io::create_subdevice(entryPoint, folder.entryPoint(), folder.pathPart());
    writeables[name] = entryPoint;
    return entryPoint;
}

void EnginePaths::setScriptFolder(std::filesystem::path folder) {
    io::set_device("script", std::make_shared<io::StdfsDevice>(folder));
    this->scriptFolder = std::move(folder);
}

void EnginePaths::setProjectFolder(std::filesystem::path folder) {
    io::set_device("project", std::make_shared<io::StdfsDevice>(folder));
    this->projectFolder = std::move(folder);
}

void EnginePaths::setCurrentWorldFolder(io::path folder) {
    if (folder.empty()) {
        io::remove_device("world");
    } else {
        io::create_subdevice("world", "user", folder);
    }
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

std::vector<io::path> EnginePaths::scanForWorlds() const {
    std::vector<io::path> folders;

    auto folder = getWorldsFolder();
    if (!io::is_directory(folder)) return folders;

    for (const auto& worldFolder : io::directory_iterator(folder)) {
        if (!io::is_directory(worldFolder)) continue;

        auto worldFile = worldFolder / WorldFiles::WORLD_FILE;
        if (!io::is_regular_file(worldFile)) continue;
        folders.push_back(worldFolder);
    }

    std::sort(
        folders.begin(), folders.end(),
        [](io::path a, io::path b) {
            a = a / WorldFiles::WORLD_FILE;
            b = b / WorldFiles::WORLD_FILE;
            return std::filesystem::last_write_time(io::resolve(a)) > std::filesystem::last_write_time(io::resolve(b));
    });

    return folders;
}

ResPaths::ResPaths(
    std::vector<PathsRoot> roots
) : roots(std::move(roots)) {}

io::path ResPaths::find(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        auto file = root.path / filename;
        if (io::exists(file)) return file;
    }
    return io::path("res:") / filename;
}

std::string ResPaths::findRaw(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        if (io::exists(root.path / filename)) {
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
        auto folder = root.path / folderName;
        if (!io::is_directory(folder)) continue;
        for (const auto& file : io::directory_iterator(folder)) {
            entries.emplace_back(root.name + ":" + folderName + "/" + file.name());
        }
    }
    return entries;
}

std::vector<io::path> ResPaths::listdir(const std::string& folderName) const {
    std::vector<io::path> entries;
    for (int i = roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        io::path folder = root.path / folderName;
        if (!io::is_directory(folder)) continue;
        for (const auto& entry : io::directory_iterator(folder)) {
            entries.push_back(entry);
        }
    }
    return entries;
}

dv::value ResPaths::readCombinedList(const std::string& filename) const {
    dv::value list = dv::list();
    for (const auto& root : roots) {
        auto path = root.path / filename;
        if (!io::exists(path)) continue;

        try {
            auto value = io::read_object(path);
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

dv::value ResPaths::readCombinedObject(const std::string& filename, bool deep) const {
    dv::value object = dv::object();
    for (const auto& root : roots) {
        auto path = root.path / filename;
        if (!io::exists(path)) continue;

        try {
            auto value = io::read_object(path);
            if (!value.isObject()) {
                LOG_WARN("Reading combined object {}: is not an object (skipped)", root.name, filename);
            }
            object.merge(std::move(value), deep);
        } catch (const std::runtime_error& err) {
            LOG_WARN("Reading combined object {}: {}: {}", root.name, filename, err.what());
        }
    }
    return object;
}

std::vector<io::path> ResPaths::collectRoots() {
    std::vector<io::path> collected;
    collected.reserve(roots.size());
    for (const auto& root : roots) {
        collected.emplace_back(root.path);
    }
    return collected;
}
