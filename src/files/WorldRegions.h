#ifndef FILES_WORLD_REGIONS_H_
#define FILES_WORLD_REGIONS_H_

#include <mutex>
#include <memory>
#include <functional>
#include <filesystem>
#include <unordered_map>
#include <condition_variable>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "files.h"
#include "../typedefs.h"
#include "../util/BufferPool.h"
#include "../voxels/Chunk.h"

namespace RegionConsts {
    inline constexpr uint SIZE_BIT = 5; // Размер региона 
    inline constexpr uint SIZE = 1 << SIZE_BIT; // Длина региона в чанках
    inline constexpr uint VOLUME = SIZE * SIZE; // Количество чанков в регионе

    inline constexpr uint LAYER_VOXELS = 0;
    inline constexpr uint LAYER_LIGHTS = 1;
    inline constexpr uint LAYER_INVENTORIES = 2;

    inline constexpr uint MAX_OPEN_FILES = 16;

    inline constexpr int FORMAT_VERSION = 2;
}

class illegal_region_format : public std::runtime_error {
public:
    illegal_region_format(const std::string& message) 
    : std::runtime_error(message) {}
};

class WorldRegion {
    ubyte** chunksData;
    uint32_t* sizes;
    bool unsaved = false;
public:
    WorldRegion();
    ~WorldRegion();

    void put(uint x, uint z, ubyte* data, uint32_t size);
    ubyte* getChunkData(uint x, uint z);
    uint getChunkDataSize(uint x, uint z);

    void setUnsaved(bool unsaved);
    bool isUnsaved() const;

    ubyte** getChunks() const;
    uint32_t* getSizes() const;
};

struct regFile {
    files::rafile file;
    int version;
    bool inUse = false;

    regFile(std::filesystem::path filename);
    regFile(const regFile&) = delete;

    std::unique_ptr<ubyte[]> read(int index, uint32_t& length);
};

using regionsmap = std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>>;
using regionproc = std::function<bool(ubyte*)>;

struct RegionsLayer {
    int layer;
    std::filesystem::path folder;
    regionsmap regions;
    std::mutex mutex;
};

class WorldRegions {
    std::filesystem::path directory;
    std::unordered_map<glm::ivec3, std::unique_ptr<regFile>> openRegFiles;
    std::mutex regFilesMutex;
    std::condition_variable regFilesCv;
    RegionsLayer layers[3] {};
    util::BufferPool<ubyte> bufferPool {
        std::max(CHUNK_DATA_LEN, LIGHTMAP_DATA_LEN) * 2
    };

    WorldRegion* getRegion(int x, int z, int layer);
    WorldRegion* getOrCreateRegion(int x, int z, int layer);

    std::unique_ptr<ubyte[]> compress(const ubyte* src, size_t srclen, size_t& len);

    std::unique_ptr<ubyte[]> decompress(const ubyte* src, size_t srclen, size_t dstlen);

    std::unique_ptr<ubyte[]> readChunkData(int x, int z, uint32_t& length, regFile* file);

    void fetchChunks(WorldRegion* region, int x, int z, regFile* file);

    ubyte* getData(int x, int z, int layer, uint32_t& size);

    std::shared_ptr<regFile> getRegFile(glm::ivec3 coord, bool create=true);
    void closeRegFile(glm::ivec3 coord);
    std::shared_ptr<regFile> useRegFile(glm::ivec3 coord);
    std::shared_ptr<regFile> createRegFile(glm::ivec3 coord);

    std::filesystem::path getRegionFilename(int x, int y) const;

    void writeRegions(int layer);

    void writeRegion(int x, int z, int layer, WorldRegion* entry);
public:
    bool generatorTestMode = false;
    bool doWriteLights = true;

    WorldRegions(std::filesystem::path directory);
    WorldRegions(const WorldRegions&) = delete;
    ~WorldRegions();

    void put(Chunk* chunk);

    void put(int x, int z, int layer, std::unique_ptr<ubyte[]> data, size_t size, bool rle);

    std::unique_ptr<ubyte[]> getChunk(int x, int z);
    std::unique_ptr<light_t[]> getLights(int x, int z);
    chunk_inventories_map fetchInventories(int x, int z);

    void processRegionVoxels(int x, int z, regionproc func);

    std::filesystem::path getRegionsFolder(int layer) const;

    void write();

    static bool parseRegionFilename(const std::string& name, int& x, int& z);
};

#endif // FILES_WORLD_REGIONS_H_
