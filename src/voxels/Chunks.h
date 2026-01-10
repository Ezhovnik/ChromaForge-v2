#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>

#include <glm/glm.hpp>

#include "../typedefs.h"

class Mesh;
class VoxelRenderer;

class Chunk;
class voxel;
class WorldFiles;

// Класс для управления набором чанков в воксельном мире.
class Chunks{
public:
    Chunk** chunks;
    Chunk** chunksSecond;
    Mesh** meshes;
    Mesh** meshesSecond;

    size_t volume;
    size_t chunksCount;

    uint width; // Количество чанков по X
    uint depth; // Количество чанков по Z
    int areaOffsetX; // Смещение области видимых чанков по X
    int areaOffsetZ; // Смещение области видимых чанков по Z

    Chunks(uint width, uint depth, int areaOffsetX, int areaOffsetZ); // Конструктор
    ~Chunks(); // Деструктор

    bool putChunk(Chunk* chunk);

    Chunk* getChunk(int chunk_x, int chunk_z); // Возвращает чанк по координатам чанка
    Chunk* getChunkByVoxel(int x, int y, int z); // Получает чанк, содержащий воксель с заданными мировыми координатами

    voxel* getVoxel(int x, int y, int z); // Возвращает воксель по мировым координатам
    void setVoxel(int x, int y, int z, int id, uint8_t states); // Устанавливает идентификатор вокселя по мировым координатам

    ubyte getLight(int x, int y, int z);
    ubyte getLight(int x, int y, int z, int channel); // Получает уровень освещения вокселя
    
    voxel* rayCast( // Выполняет трассировку луча через воксельный мир.
        glm::vec3 start, // Начальная точка луча
        glm::vec3 dir, // Направление луча
        float maxDist, // Максимальная дистанция трассировки
        glm::vec3& end, // Точка попадания луча
        glm::vec3& norm, // Нормаль поверхности в точке попадания
        glm::vec3& iend // Координаты вокселя в точке попадания
    );

    bool isObstacle(int x, int y, int z);

    void _setOffset(int x, int z);

    void setCenter(WorldFiles* worldFiles, int x, int z);
	void translate(WorldFiles* worldFiles, int x, int z);

    void clear(bool freeMemory);
};

#endif // VOXELS_CHUNKS_H_
