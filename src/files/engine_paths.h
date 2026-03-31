#ifndef FILES_ENGINE_PATHS_H_
#define FILES_ENGINE_PATHS_H_

#include <string>
#include <filesystem>
#include <vector>
#include <stdexcept>

#include "../content/ContentPack.h"

class files_access_error : public std::runtime_error {
public:
    files_access_error(const std::string& msg) : std::runtime_error(msg) {}
};

class EnginePaths {
private:
    std::filesystem::path userfiles {"."};
    std::filesystem::path resources {"res"};
    std::filesystem::path worldFolder;
    std::vector<ContentPack>* contentPacks = nullptr;
public:
    std::filesystem::path getUserfiles() const;
    std::filesystem::path getResources() const;
    
    std::filesystem::path getScreenshotFile(std::string ext);
    std::filesystem::path getWorldsFolder();
    std::filesystem::path getWorldFolder();
    std::filesystem::path getWorldFolder(const std::string& name);
    std::filesystem::path getLogsFile();
    std::filesystem::path getControlsFile();
    std::filesystem::path getSettingsFile();
    std::filesystem::path getBindingsFile(const std::string& folder="");
    bool isWorldNameUsed(std::string name);

    void setUserfiles(std::filesystem::path folder);
    void setResources(std::filesystem::path folder);
    void setContentPacks(std::vector<ContentPack>* contentPacks);
    void setWorldFolder(std::filesystem::path folder);

    std::vector<std::filesystem::path> scanForWorlds();

    std::filesystem::path resolve(std::string path);
};

struct PathsRoot {
    std::string name;
    std::filesystem::path path;
};

class ResPaths {
private:
    std::filesystem::path mainRoot;
    std::vector<PathsRoot> roots;
public:
    ResPaths(
        std::filesystem::path mainRoot,
        std::vector<PathsRoot> roots
    );

    std::filesystem::path find(const std::string& filename) const;
    std::string findRaw(const std::string& filename) const;
    std::vector<std::filesystem::path> listdir(const std::string& folder) const;
    std::vector<std::string> listdirRaw(const std::string& folder) const;

    const std::filesystem::path& getMainRoot() const;
};

#endif // FILES_ENGINE_PATHS_H_
