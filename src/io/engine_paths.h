#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <optional>
#include <tuple>

#include <content/ContentPack.h>
#include <data/dv.h>
#include <io/io.h>

class EnginePaths {
private:
    std::filesystem::path userFilesFolder {"."};
    std::filesystem::path resourcesFolder {"res"};
    io::path currentWorldFolder;
    std::optional<std::filesystem::path> scriptFolder;
    std::vector<ContentPack>* contentPacks = nullptr;
    std::vector<std::string> contentEntryPoints;
    std::unordered_map<std::string, std::string> writeablePacks;
    std::vector<std::string> mounted;
public:
    void prepare();

    void setUserFilesFolder(std::filesystem::path folder);
    const std::filesystem::path& getUserFilesFolder() const;

    void setResourcesFolder(std::filesystem::path folder);
    const std::filesystem::path& getResourcesFolder() const;

    void setScriptFolder(std::filesystem::path folder);

    io::path getConfigFolder() const;
    io::path getWorldsFolder() const;
    io::path getWorldFolderByName(const std::string& name);

    void setCurrentWorldFolder(io::path folder);
    io::path getCurrentWorldFolder();

    io::path getNewScreenshotFile(const std::string& ext);
    io::path getControlsFile() const;
    io::path getSettingsFile() const;

    std::string mount(const io::path& file);
    void unmount(const std::string& name);

    void setContentPacks(std::vector<ContentPack>* contentPacks);

    std::string createWriteablePackDevice(const std::string& name);

    std::vector<io::path> scanForWorlds() const;

    static std::tuple<std::string, std::string> parsePath(std::string_view view);

    static inline io::path CONFIG_DEFAULTS = "config/defaults.toml";
};

struct PathsRoot {
    std::string name;
    io::path path;
};

class ResPaths {
private:
    io::path mainRoot;
    std::vector<PathsRoot> roots;
public:
    ResPaths(
        io::path mainRoot,
        std::vector<PathsRoot> roots
    );

    io::path find(const std::string& filename) const;
    std::string findRaw(const std::string& filename) const;
    std::vector<io::path> listdir(const std::string& folder) const;
    std::vector<std::string> listdirRaw(const std::string& folder) const;

    dv::value readCombinedList(const std::string& file) const;

    dv::value readCombinedObject(const std::string& file, bool deep=false) const;

    const io::path& getMainRoot() const;
};
