#pragma once

#include <stdlib.h>
#include <unordered_map>
#include <memory>

#include <typedefs.h>
#include <constants.h>
#include <voxels/voxel.h>
#include <util/SmallHeap.h>
#include <math/AABB.h>

inline constexpr int CHUNK_DATA_LEN = CHUNK_VOLUME * 4;

class Inventory;
class ContentReport;
class Lightmap;

using ChunkInventoriesMap = std::unordered_map<uint, std::shared_ptr<Inventory>>;
using BlocksMetadata = util::SmallHeap<uint16_t, uint8_t>;

// Чанк - часть воксельного мира
class Chunk {
public:
    int chunk_x, chunk_z; // Координаты чанка
    int bottom, top;
    voxel voxels[CHUNK_VOLUME] {}; // Массив вокселей, содержащихся в чанке

    struct {
        bool modified: 1; // is chunk mesh should be updated
        bool ready: 1; // is chunk ready for modifications (loaded / generated)
        bool loaded: 1; // was chunk loaded from region
        bool lighted: 1; // is lights built (chunk ready to be visualized)
        bool unsaved: 1; // does chunk contain unsaved changes
        bool loadedLights: 1; // was lights loaded from cache
        bool entities: 1; // does chunk contain entities list changes
        bool blocksData: 1; // does chunk contain block fields changes
        bool dirtyHeights : 1; // is chunk bottom, top should be recalculated
        bool inventoriesRemoved : 1; // was block inventories removed since the last save
    } flags {};

    std::shared_ptr<Lightmap> lightmap; // Карта освещения чанка

    uint64_t lastRandomSparkId = -1;

    ChunkInventoriesMap inventories;

    BlocksMetadata blocksMetadata;

    Chunk(
        int chunk_x,
        int chunk_z,
        std::shared_ptr<Lightmap> lightmap = nullptr
    ); // Конструктор

    void updateHeights();

    void addBlockInventory(std::shared_ptr<Inventory> inventory, uint x, uint y, uint z);
    std::shared_ptr<Inventory> getBlockInventory(uint x, uint y, uint z) const;
    void removeBlockInventory(uint x, uint y, uint z);
	void setBlockInventories(ChunkInventoriesMap map);

    inline void setModifiedAndUnsaved() {
        flags.modified = true;
        flags.unsaved = true;
    }

    std::unique_ptr<ubyte[]> encode() const;
	bool decode(const ubyte* data);

    static void convert(ubyte* data, const ContentReport* report);

    AABB getAABB() const {
        return AABB(
            glm::vec3(chunk_x * CHUNK_WIDTH, -INFINITY, chunk_z * CHUNK_DEPTH),
            glm::vec3((chunk_x + 1) * CHUNK_WIDTH, INFINITY, (chunk_z + 1) * CHUNK_DEPTH)
        );
    }

    bool isBlockInside(int x, int z) const {
        x -= this->chunk_x * CHUNK_WIDTH;
        z -= this->chunk_z * CHUNK_DEPTH;
        return x >= 0 && z >= 0 && x < CHUNK_WIDTH && z < CHUNK_DEPTH;
    }
};
