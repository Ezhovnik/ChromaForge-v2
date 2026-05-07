#pragma once

#include <stdlib.h>
#include <unordered_map>
#include <memory>

#include <typedefs.h>
#include <constants.h>
#include <voxels/voxel.h>
#include <lighting/LightMap.h>
#include <util/SmallHeap.h>

inline constexpr int CHUNK_DATA_LEN = CHUNK_VOLUME * 4;

class Inventory;
class ContentReport;

using ChunkInventoriesMap = std::unordered_map<uint, std::shared_ptr<Inventory>>;
using BlocksMetadata = util::SmallHeap<uint16_t, uint8_t>;

// Чанк - часть воксельного мира
class Chunk {
public:
    int chunk_x, chunk_z; // Координаты чанка
    int bottom, top;
    voxel voxels[CHUNK_VOLUME] {}; // Массив вокселей, содержащихся в чанке

    struct {
        bool modified: 1;
        bool ready: 1;
        bool loaded: 1;
        bool lighted: 1;
        bool unsaved: 1;
        bool loadedLights: 1;
        bool entities: 1;
        bool blocksData: 1;
    } flags {};

    LightMap light_map; // Карта освещения чанка

    ChunkInventoriesMap inventories;

    BlocksMetadata blocksMetadata;

    Chunk(int chunk_x, int chunk_z); // Конструктор

    bool isEmpty(); // Проверяет, является ли чанк пустым (однородным).

    void updateHeights();

    std::unique_ptr<Chunk> clone() const; // Создает полную копию текущего чанка.

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
};
