#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>

#include "../typedefs.h"
#include "../constants.h"

namespace ChunkFlags {
    inline constexpr uint MODIFIED = 0x1;
    inline constexpr uint READY = 0x2;
    inline constexpr uint LOADED = 0x4;
    inline constexpr uint LIGHTED = 0x8;
    inline constexpr uint UNSAVED = 0x10;
    inline constexpr uint LOADED_LIGHTS = 0x20;
}

inline constexpr int CHUNK_DATA_LEN = CHUNK_VOLUME * 2;

struct voxel;
class LightMap;

struct RenderData {
    float* vertices;
    size_t size;
};

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
    voxel* voxels; // Массив вокселей, содержащихся в чанке

    int surrounding = 0; // Счётчик окружающих, загруженных чанков
    int flags = 0;

    RenderData renderData;

    LightMap* light_map; // Карта освещения чанка

    Chunk(int chunk_x, int chunk_z); // Конструктор
    ~Chunk(); // Деструктор

    bool isEmpty(); // Проверяет, является ли чанк пустым (однородным).

    void updateHeights();

    Chunk* clone() const; // Создает полную копию текущего чанка.

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
};

#endif // VOXELS_CHUNK_H_
