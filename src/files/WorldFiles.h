#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <memory>
#include <filesystem>
#include <vector>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <typedefs.h>
#include "files/files.h"
#include <voxels/Chunk.h>
#include "content/ContentPack.h"
#include "WorldRegions.h"

class Player;
class Content;
class ContentIndices;
class World;
struct DebugSettings;

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
private:
    std::filesystem::path directory; // Путь к директории с файлами мира
    WorldRegions regions;

    bool generatorTestMode = false;
    bool doWriteLights = true;

    std::filesystem::path getWorldFile() const; // Генерирует имя файла, в котором записана общая информация о мире
    std::filesystem::path getIndicesFile() const;
    std::filesystem::path getPacksFile() const;

    void writeWorldInfo(const World* world);
    void writeIndices(const ContentIndices* indices);
public:
    static const inline std::string WORLD_FILE = "world.json";

    WorldFiles(const std::filesystem::path& directory);
    WorldFiles(const std::filesystem::path& directory, const DebugSettings& settings); // Конструктор
    ~WorldFiles(); // Деструктор

    std::filesystem::path getPlayerFile() const;
    std::filesystem::path getResourcesFile() const;
    void createDirectories();

    bool readWorldInfo(World* world);

    bool readResourcesData(const Content* content);

    void write(const World* world, const Content* content);
    void writePacks(const std::vector<ContentPack>& packs);

    void removeIndices(const std::vector<std::string>& packs);

    std::filesystem::path getFolder() const;

    WorldRegions& getRegions() {
        return regions;
    }

    bool doesWriteLights() const {
        return doWriteLights;
    }
};

#endif // FILES_WORLDFILES_H_
