#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <typedefs.h>
#include <io/io.h>
#include <voxels/Chunk.h>
#include <content/ContentPack.h>
#include <world/files/WorldRegions.h>

class Player;
class Content;
class ContentIndices;
class World;
struct WorldInfo;
struct DebugSettings;

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
private:
    io::path directory; // Путь к директории с файлами мира
    WorldRegions regions;

    bool generatorTestMode = false;
    bool doWriteLights = true;

    io::path getWorldFile() const; // Генерирует имя файла, в котором записана общая информация о мире
    io::path getPacksFile() const;

    void writeWorldInfo(const WorldInfo& info);
    void writeIndices(const ContentIndices* indices);
public:
    static const inline std::string WORLD_FILE = "world.json";

    WorldFiles(const io::path& directory);
    WorldFiles(const io::path& directory, const DebugSettings& settings); // Конструктор
    ~WorldFiles(); // Деструктор

    io::path getPlayerFile() const;
    io::path getResourcesFile() const;
    io::path getIndicesFile() const;
    void createDirectories();

    std::optional<WorldInfo> readWorldInfo();

    bool readResourcesData(const Content& content);

    static void createContentIndicesCache(
        const ContentIndices* indices, dv::value& root
    );
    static void createBlockFieldsIndices(
        const ContentIndices* indices, dv::value& root
    );

    void patchIndicesFile(const dv::value& map);

    void write(const World* world, const Content* content);
    void writePacks(const std::vector<ContentPack>& packs);

    void removeIndices(const std::vector<std::string>& packs);

    io::path getFolder() const;

    WorldRegions& getRegions() {
        return regions;
    }

    bool doesWriteLights() const {
        return doWriteLights;
    }
};
