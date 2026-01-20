#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <memory>
#include <stdlib.h>

#include <glm/glm.hpp>

#include "../typedefs.h"

class Mesh;
class VoxelRenderer;

class Chunk;
class voxel;
class WorldFiles;
class LevelEvents;

// Класс для управления набором чанков в воксельном мире.
class Chunks{
public:
    std::shared_ptr<Chunk>* chunks;
    std::shared_ptr<Chunk>* chunksSecond;

    size_t volume;
    size_t chunksCount;

    uint width; // Количество чанков по X
    uint depth; // Количество чанков по Z
    int areaOffsetX; // Смещение области видимых чанков по X
    int areaOffsetZ; // Смещение области видимых чанков по Z

    WorldFiles* worldFiles;

    LevelEvents* events;

    Chunks(uint width, uint depth, int areaOffsetX, int areaOffsetZ, WorldFiles* worldFiles, LevelEvents* events); // Конструктор
    ~Chunks(); // Деструктор

    bool putChunk(std::shared_ptr<Chunk> chunk);

    Chunk* getChunk(int chunk_x, int chunk_z); // Возвращает чанк по координатам чанка
    Chunk* getChunkByVoxel(int x, int y, int z); // Получает чанк, содержащий воксель с заданными мировыми координатами

    voxel* getVoxel(int x, int y, int z); // Возвращает воксель по мировым координатам
    void setVoxel(int x, int y, int z, blockid_t id, uint8_t states); // Устанавливает идентификатор вокселя по мировым координатам

    light_t getLight(int x, int y, int z);
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

    void setCenter(int x, int z);
	void translate(int x, int z);
    void resize(uint newWidth, uint newDepth);

    void clear();
};

#endif // VOXELS_CHUNKS_H_
