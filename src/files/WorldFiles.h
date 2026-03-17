#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <vector>
#include <condition_variable>
#include <mutex>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"
#include "../settings.h"
#include "../files/files.h"
#include "../voxels/Chunk.h"
#include "../content/ContentPack.h"

// Константы для размера регионов
namespace RegionConsts {
    inline constexpr uint SIZE_BIT = 5; // Размер региона 
    inline constexpr uint SIZE = 1 << SIZE_BIT; // Длина региона в чанках
    inline constexpr uint VOLUME = SIZE * SIZE; // Количество чанков в регионе

    inline constexpr uint LAYER_VOXELS = 0;
    inline constexpr uint LAYER_LIGHTS = 1;
    inline constexpr uint LAYER_INVENTORIES = 2;

    inline constexpr uint MAX_OPEN_FILES = 16;
}

inline constexpr int REGION_FORMAT_VERSION = 2;

class Player;
class Content;
class ContentIndices;
class World;

class illegal_region_format : public std::runtime_error {
public:
    illegal_region_format(const std::string& message) : std::runtime_error(message) {}
};

class WorldRegion {
private:
	ubyte** chunksData;
	uint32_t* sizes;
	bool unsaved = false;
public:
	WorldRegion();
	~WorldRegion();

	void put(uint x, uint z, ubyte* data, uint32_t size);
	ubyte* getChunkData(uint x, uint z);
	uint getChunkDataSize(uint x, uint z);

    void setUnsaved(bool unsaved) {this->unsaved = unsaved;};
	bool isUnsaved() const {return unsaved;};

	ubyte** getChunks() const {return chunksData;};
	uint32_t* getSizes() const {return sizes;};
};

struct regFile {
    files::rafile file;
    int version;
    bool inUse = false;

    regFile(std::filesystem::path filename);
};

typedef std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>> regionsmap;

// Класс для управления хранением и загрузкой данных мира в формате чанков и регионов.
class WorldFiles {
private:
    std::unordered_map<glm::ivec3, std::unique_ptr<regFile>> openRegFiles;
    std::mutex regFilesMutex;
    std::condition_variable regFilesCv;

    std::filesystem::path getLightsFolder() const;
    std::filesystem::path getInventoriesFolder() const;
	std::filesystem::path getRegionFilename(int x, int z) const;
    std::filesystem::path getWorldFile() const; // Генерирует имя файла, в котором записана общая информация о мире
    std::filesystem::path getIndicesFile() const;
    std::filesystem::path getPacksFile() const;

    ubyte* compress(const ubyte* src, size_t srclen, size_t& len);
    ubyte* decompress(const ubyte* src, size_t srclen, size_t dstlen);

    void writeWorldInfo(const World* world);
    void writeRegions(regionsmap& regions, const std::filesystem::path& folder, int layer);

    void fetchChunks(WorldRegion* region, int x, int y, regFile* file);

    ubyte* readChunkData(int x, int y, uint32_t& length, regFile* file);
    WorldRegion* getRegion(regionsmap& regions, int x, int z);
    WorldRegion* getOrCreateRegion(regionsmap& regions, int x, int z);
	ubyte* getData(regionsmap& regions, const std::filesystem::path& folder, int x, int z, int layer, bool compression);

    std::shared_ptr<regFile> getRegFile(glm::ivec3 coord, const std::filesystem::path& folder);
    void closeRegFile(glm::ivec3 coord);
    std::shared_ptr<regFile> useRegFile(glm::ivec3 coord);
    std::shared_ptr<regFile> createRegFile(glm::ivec3 coord, const std::filesystem::path& folder);
public:
    regionsmap regions; // Хранилище регионов в оперативной памяти.
    regionsmap lights;
    regionsmap storages;

    std::filesystem::path directory; // Путь к директории с файлами мира
    std::unique_ptr<ubyte[]> compressionBuffer; // Выходной буфер для записи регионов

    bool generatorTestMode;
    bool doWriteLights;

    static const inline std::string WORLD_FILE = "world.json";

    WorldFiles(std::filesystem::path directory, const DebugSettings& settings); // Конструктор
    ~WorldFiles(); // Деструктор

    void createDirectories();

    static bool parseRegionFilename(const std::string& name, int& x, int& z);
    std::filesystem::path getRegionsFolder() const;
    std::filesystem::path getPlayerFile() const; // Генерирует имя файла, в котором записана информация об игроке

    void put(Chunk* chunk); // Сохраняет данные чанка в кэш памяти.
    void put(int x, int z, const ubyte* voxelData);

    bool readWorldInfo(World* world);
	void writeRegion(int x, int z, WorldRegion* entry, std::filesystem::path file, int layer);

	ubyte* getChunk(int x, int z); // Получает данные чанка из кэша или файла
    light_t* getLights(int x, int z);

    chunk_inventories_map fetchInventories(int x, int z);

    void write(const World* world, const Content* content);
    void writePacks(const std::vector<ContentPack>& packs);
    void writeIndices(const ContentIndices* indices);

    void removeIndices(const std::vector<std::string>& packs);
};

#endif // FILES_WORLDFILES_H_
