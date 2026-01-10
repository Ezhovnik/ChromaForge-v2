#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>

#include "../typedefs.h"

// Размеры чанка
inline constexpr int CHUNK_WIDTH = 16; // Ширина по X
inline constexpr int CHUNK_HEIGHT = 256; // Высота по Y
inline constexpr int CHUNK_DEPTH = 16; // Глубина по Z
inline constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; // Общее количество вокселей в чанке

namespace Chunk_Flags {
    inline constexpr uint MODIFIED = 0x1;
    inline constexpr uint READY = 0x2;
    inline constexpr uint LOADED = 0x4;
    inline constexpr uint LIGHTED = 0x8;
    inline constexpr uint UNSAVED = 0x10;
}

class voxel;
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
    voxel* voxels; // Массив вокселей, содержащихся в чанке

    int surrounding = 0; // Счётчик окружающих, загруженных чанков
    int references = 1; // Счётчик ссылок
    int flags = 0;

    RenderData renderData;

    LightMap* light_map; // Карта освещения чанка

    Chunk(int chunk_x, int chunk_z); // Конструктор
    ~Chunk(); // Деструктор

    bool isEmpty(); // Проверяет, является ли чанк пустым (однородным).

    Chunk* clone() const; // Создает полную копию текущего чанка.

    void incref(); // Увеличивает счетчик ссылок на чанк.
    void decref(); // Уменьшает счётчик ссылок на чанк

    inline bool isUnsaved() const {return flags & Chunk_Flags::UNSAVED;}
	inline bool isModified() const {return flags & Chunk_Flags::MODIFIED;}
	inline bool isLighted() const {return flags & Chunk_Flags::LIGHTED;}
	inline bool isLoaded() const {return flags & Chunk_Flags::LOADED;}
	inline bool isReady() const {return flags & Chunk_Flags::READY;}

	inline void setUnsaved(bool flag) {bitset(flags, Chunk_Flags::UNSAVED, flag);}
	inline void setModified(bool flag) {bitset(flags, Chunk_Flags::MODIFIED, flag);}
	inline void setLoaded(bool flag) {bitset(flags, Chunk_Flags::LOADED, flag);}
	inline void setLighted(bool flag) {bitset(flags, Chunk_Flags::LIGHTED, flag);}
	inline void setReady(bool flag) {bitset(flags, Chunk_Flags::READY, flag);}
};

#endif // VOXELS_CHUNK_H_
