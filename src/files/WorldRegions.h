#pragma once

#include <mutex>
#include <memory>
#include <functional>
#include <filesystem>
#include <unordered_map>
#include <condition_variable>
#include <vector>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <files/files.h>
#include <typedefs.h>
#include <util/BufferPool.h>
#include <voxels/Chunk.h>
#include <data/dynamic_fwd.h>
#include <math/voxmaths.h>
#include <coders/compression.h>
#include <files/world_regions_fwd.h>

namespace RegionConsts {
    inline constexpr uint SIZE_BIT = 5; // Размер региона 
    inline constexpr uint SIZE = 1 << SIZE_BIT; // Длина региона в чанках
    inline constexpr uint VOLUME = SIZE * SIZE; // Количество чанков в регионе
}

class illegal_region_format : public std::runtime_error {
public:
    illegal_region_format(const std::string& message) 
    : std::runtime_error(message) {}
};

class WorldRegion {
private:
    std::unique_ptr<std::unique_ptr<ubyte[]>[]> chunksData;
    std::unique_ptr<glm::u32vec2[]> sizes;
    bool unsaved = false;
public:
    WorldRegion();
    ~WorldRegion();

    void put(
        uint x, uint z,
        std::unique_ptr<ubyte[]> data,
        uint32_t size,
        uint32_t srcSize
    );
    ubyte* getChunkData(uint x, uint z);
    glm::u32vec2 getChunkDataSize(uint x, uint z);

    void setUnsaved(bool unsaved);
    bool isUnsaved() const;

    std::unique_ptr<ubyte[]>* getChunks() const;
    glm::u32vec2* getSizes() const;
};

struct regFile {
    files::rafile file;
    int version;
    bool inUse = false;

    regFile(std::filesystem::path filename);
    regFile(const regFile&) = delete;

    std::unique_ptr<ubyte[]> read(int index, uint32_t& size, uint32_t& srcSize);
};

class regFile_ptr {
private:
    regFile* file;
    std::condition_variable* cv;
public:
    regFile_ptr(
        regFile* file,
        std::condition_variable* cv
    ) : file(file), cv(cv) {}

    regFile_ptr(const regFile_ptr&) = delete;

    regFile_ptr(std::nullptr_t) : file(nullptr), cv(nullptr) {}

    bool operator==(std::nullptr_t) const {
        return file == nullptr;
    }
    bool operator!=(std::nullptr_t) const {
        return file != nullptr;
    }
    operator bool() const {
        return file != nullptr;
    }
    ~regFile_ptr() {
        reset();
    }
    regFile* get() {
        return file;
    }
    void reset() {
        if (file) {
            file->inUse = false;
            cv->notify_one();
            file = nullptr;
        }
    }
};

using regionsmap = std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>>;
using regionproc = std::function<std::unique_ptr<ubyte[]>(std::unique_ptr<ubyte[]>,uint32_t*)>;
using inventoryproc = std::function<void(Inventory*)>;

inline void calc_reg_coords(
    int x, int z, int& regionX, int& regionZ, int& localX, int& localZ
) {
    regionX = floordiv(x, RegionConsts::SIZE);
    regionZ = floordiv(z, RegionConsts::SIZE);
    localX = x - (regionX * RegionConsts::SIZE);
    localZ = z - (regionZ * RegionConsts::SIZE);
}

struct RegionsLayer {
    RegionLayerIndex layer;
    std::filesystem::path folder;
    compression::Method compression = compression::Method::None;
    regionsmap regions;
    std::mutex mapMutex;
    std::unordered_map<glm::ivec2, std::unique_ptr<regFile>> openRegFiles;
    std::mutex regFilesMutex;
    std::condition_variable regFilesCv;

    [[nodiscard]] regFile_ptr getRegFile(glm::ivec2 coord, bool create = true);
    [[nodiscard]] regFile_ptr useRegFile(glm::ivec2 coord);
    regFile_ptr createRegFile(glm::ivec2 coord);
    void closeRegFile(glm::ivec2 coord);

    WorldRegion* getRegion(int x, int z);
    WorldRegion* getOrCreateRegion(int x, int z);

    std::filesystem::path getRegionFilePath(int x, int z) const;

    [[nodiscard]] ubyte* getData(int x, int z, uint32_t& size, uint32_t& srcSize);

    void writeRegion(int x, int y, WorldRegion* entry);
    void writeAll();

    [[nodiscard]] static std::unique_ptr<ubyte[]> readChunkData(
        int x, int z, uint32_t& size, uint32_t& srcSize, regFile* rfile
    );
};

class WorldRegions {
    std::filesystem::path directory;
    RegionsLayer layers[REGION_LAYERS_COUNT] {};
public:
    bool generatorTestMode = false;
    bool doWriteLights = true;

    WorldRegions(const std::filesystem::path& directory);
    WorldRegions(const WorldRegions&) = delete;
    ~WorldRegions();

    void put(Chunk* chunk, std::vector<ubyte> entitiesData);

    void put(
        int x, int z,
        RegionLayerIndex layer,
        std::unique_ptr<ubyte[]> data,
        size_t size
    );

    std::unique_ptr<ubyte[]> getVoxels(int x, int z);
    std::unique_ptr<light_t[]> getLights(int x, int z);
    chunk_inventories_map fetchInventories(int x, int z);
    dynamic::Map_sptr fetchEntities(int x, int z);

    void processRegion(
        int x, int z, RegionLayerIndex layerID, const regionproc& func
    );

    void processInventories(
        int x, int z, const inventoryproc& func
    );

    const std::filesystem::path& getRegionsFolder(RegionLayerIndex layerID) const;
    std::filesystem::path getRegionFilePath(RegionLayerIndex layerID, int x, int z) const;

    void writeAll();

    static bool parseRegionFilename(const std::string& name, int& x, int& z);
};
