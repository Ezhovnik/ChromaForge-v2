#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>
#include <unordered_map>
#include <memory>

#include "typedefs.h"
#include "constants.h"
#include "voxel.h"
#include "lighting/LightMap.h"

inline constexpr int CHUNK_DATA_LEN = CHUNK_VOLUME * 4;

class Inventory;
class ContentLUT;

using chunk_inventories_map = std::unordered_map<uint, std::shared_ptr<Inventory>>;

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
    } flags {};

    LightMap light_map; // Карта освещения чанка

    chunk_inventories_map inventories;

    Chunk(int chunk_x, int chunk_z); // Конструктор

    bool isEmpty(); // Проверяет, является ли чанк пустым (однородным).

    void updateHeights();

    std::unique_ptr<Chunk> clone() const; // Создает полную копию текущего чанка.

    void addBlockInventory(std::shared_ptr<Inventory> inventory, uint x, uint y, uint z);
    std::shared_ptr<Inventory> getBlockInventory(uint x, uint y, uint z) const;
    void removeBlockInventory(uint x, uint y, uint z);
	void setBlockInventories(chunk_inventories_map map);

    inline void setModifiedAndUnsaved() {
        flags.modified = true;
        flags.unsaved = true;
    }

    std::unique_ptr<ubyte[]> encode() const;
	bool decode(const ubyte* data);

    static void convert(ubyte* data, const ContentLUT* lut);
};

#endif // VOXELS_CHUNK_H_
