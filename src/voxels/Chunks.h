#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>

#include <glm/glm.hpp>

typedef unsigned int uint;

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

    size_t volume; // Общее количество чанков
    uint width; // Количество чанков по X
    uint height; // Количество чанков по Y
    uint depth; // Количество чанков по Z
    int areaOffsetX; // Смещение области видимых чанков по X
    int areaOffsetY; // Смещение области видимых чанков по Y
    int areaOffsetZ; // Смещение области видимых чанков по Z

    Chunks(uint width, uint height, uint depth, int areaOffsetX, int areaOffsetY, int areaOffsetZ); // Конструктор
    ~Chunks(); // Деструктор

    Chunk* getChunk(int chunk_x, int chunk_y, int chunk_z); // Возвращает чанк по координатам чанка
    Chunk* getChunkByVoxel(int x, int y, int z); // Получает чанк, содержащий воксель с заданными мировыми координатами

    voxel* getVoxel(int x, int y, int z); // Возвращает воксель по мировым координатам
    void setVoxel(int x, int y, int z, int id); // Устанавливает идентификатор вокселя по мировым координатам

    unsigned char getLight(int x, int y, int z, int channel); // Получает уровень освещения вокселя
    
    voxel* rayCast( // Выполняет трассировку луча через воксельный мир.
        glm::vec3 start, // Начальная точка луча
        glm::vec3 dir, // Направление луча
        float maxDist, // Максимальная дистанция трассировки
        glm::vec3& end, // Точка попадания луча
        glm::vec3& norm, // Нормаль поверхности в точке попадания
        glm::vec3& iend // Координаты вокселя в точке попадания
    );

    bool isObstacle(int x, int y, int z);

    void setCenter(WorldFiles* worldFiles, int x, int y, int z);
	void translate(WorldFiles* worldFiles, int x, int y, int z);

	bool loadVisible(WorldFiles* worldFiles);
	bool _buildMeshes(VoxelRenderer* renderer);
};

#endif // VOXELS_CHUNKS_H_
