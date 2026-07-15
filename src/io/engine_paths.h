#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <optional>
#include <tuple>

#include <data/dv.h>
#include <io/io.h>

struct PathsRoot {
    std::string name;
    io::path path;
    PathsRoot(std::string name, io::path path) : name(std::move(name)), path(std::move(path)) {}
};

class ResPaths {
public:
    ResPaths() = default;

    ResPaths(std::vector<PathsRoot> roots);

    io::path find(const std::string& filename) const;
    std::string findRaw(const std::string& filename) const;
    std::vector<io::path> listdir(const std::string& folder) const;
    std::vector<std::string> listdirRaw(const std::string& folder) const;

    dv::value readCombinedList(const std::string& file) const;

    dv::value readCombinedObject(const std::string& file, bool deep=false) const;

    std::vector<io::path> collectRoots();
private:
    std::vector<PathsRoot> roots;
};

class EnginePaths {
public:
    ResPaths resPaths;

    void prepare();

    void setUserFilesFolder(std::filesystem::path folder);
    const std::filesystem::path& getUserFilesFolder() const;

    void setResourcesFolder(std::filesystem::path folder);
    const std::filesystem::path& getResourcesFolder() const;

    void setScriptFolder(std::filesystem::path folder);

    void setProjectFolder(std::filesystem::path folder);

    io::path getWorldsFolder() const;
    io::path getWorldFolderByName(const std::string& name);

    void setCurrentWorldFolder(io::path folder);
    io::path getCurrentWorldFolder();

    io::path getNewScreenshotFile(const std::string& ext);
    io::path getNewPanoramaFolder();

    std::string mount(const io::path& file);
    void unmount(const std::string& name);

    void setEntryPoints(std::vector<PathsRoot> entryPoints);

    std::string createWriteableDevice(const std::string& name);

    std::vector<io::path> scanForWorlds() const;

    static std::tuple<std::string, std::string> parsePath(std::string_view view);

    static inline io::path CONFIG_DEFAULTS = "config/defaults.toml";
    static inline io::path CONTROLS_FILE = "user:controls.toml";
    static inline io::path SETTINGS_FILE = "user:settings.toml";
private:
    std::filesystem::path userFilesFolder {"."};
    std::filesystem::path resourcesFolder {"res"};
    std::filesystem::path projectFolder = resourcesFolder;
    io::path currentWorldFolder;
    std::optional<std::filesystem::path> scriptFolder;
    std::vector<PathsRoot> entryPoints;
    std::unordered_map<std::string, std::string> writeables;
    std::vector<std::string> mounted;

    void cleanup();
};
