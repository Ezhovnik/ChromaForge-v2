#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>
#include <unordered_map>
#include <memory>

#include "../typedefs.h"
#include "../constants.h"
#include "voxel.h"
#include "../lighting/LightMap.h"

namespace ChunkFlags {
    inline constexpr uint MODIFIED = 0x1;
    inline constexpr uint READY = 0x2;
    inline constexpr uint LOADED = 0x4;
    inline constexpr uint LIGHTED = 0x8;
    inline constexpr uint UNSAVED = 0x10;
    inline constexpr uint LOADED_LIGHTS = 0x20;
}

inline constexpr int CHUNK_DATA_LEN = CHUNK_VOLUME * 4;

class Inventory;
class ContentLUT;

inline void bit_on(int& flags, int bit) { flags |= bit; }
inline void bit_off(int& flags, int bit) { flags &= ~bit; }
inline void bitset(int& flags, int bit, bool state) {
    state ? bit_on(flags, bit) : bit_off(flags, bit);
}

// Чанк - часть воксельного мира
class Chunk {
public:
    int chunk_x, chunk_z; // Координаты чанка
    int bottom, top;
    voxel voxels[CHUNK_VOLUME]; // Массив вокселей, содержащихся в чанке

    int flags = 0;

    LightMap light_map; // Карта освещения чанка

    std::unordered_map<uint, std::shared_ptr<Inventory>> inventories;

    Chunk(int chunk_x, int chunk_z); // Конструктор

    bool isEmpty(); // Проверяет, является ли чанк пустым (однородным).

    void updateHeights();

    std::unique_ptr<Chunk> clone() const; // Создает полную копию текущего чанка.

    void addBlockInventory(std::shared_ptr<Inventory> inventory, uint x, uint y, uint z);
    std::shared_ptr<Inventory> getBlockInventory(uint x, uint y, uint z) const;

    inline bool isUnsaved() const {return flags & ChunkFlags::UNSAVED;}
	inline bool isModified() const {return flags & ChunkFlags::MODIFIED;}
	inline bool isLighted() const {return flags & ChunkFlags::LIGHTED;}
	inline bool isLoaded() const {return flags & ChunkFlags::LOADED;}
	inline bool isReady() const {return flags & ChunkFlags::READY;}
    inline bool isLoadedLights() const {return flags & ChunkFlags::LOADED_LIGHTS;}

	inline void setUnsaved(bool flag) {bitset(flags, ChunkFlags::UNSAVED, flag);}
	inline void setModified(bool flag) {bitset(flags, ChunkFlags::MODIFIED, flag);}
	inline void setLoaded(bool flag) {bitset(flags, ChunkFlags::LOADED, flag);}
	inline void setLighted(bool flag) {bitset(flags, ChunkFlags::LIGHTED, flag);}
	inline void setReady(bool flag) {bitset(flags, ChunkFlags::READY, flag);}
    inline void setLoadedLights(bool flag) {bitset(flags, ChunkFlags::LOADED_LIGHTS, flag);}

    ubyte* encode() const;
	bool decode(ubyte* data);

    static void fromOld(ubyte* data);
    static void convert(ubyte* data, const ContentLUT* lut);
};

#endif // VOXELS_CHUNK_H_
