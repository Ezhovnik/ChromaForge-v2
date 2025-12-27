#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>

#include <glm/glm.hpp>

typedef unsigned int uint;

class Chunk;
class voxel;

// Класс для управления набором чанков в воксельном мире.
class Chunks{
public:
    Chunk** chunks; // Массив указателей на чанки набора
    size_t volume; // Общее количество чанков
    uint width; // Количество чанков по X
    uint height; // Количество чанков по Y
    uint depth; // Количество чанков по Z

    Chunks(uint width, uint height, uint depth); // Конструктор
    ~Chunks(); // Деструктор

    Chunk* getChunk(int chunk_x, int chunk_y, int chunk_z); // Возвращает чанк по координатам чанка
    voxel* getVoxel(int x, int y, int z); // Возвращает воксель по мировым координатам
    void setVoxel(int x, int y, int z, int id); // Устанавливает идентификатор вокселя по мировым координатам
    Chunk* getChunkByVoxel(int x, int y, int z);

    unsigned char getLight(int x, int y, int z, int channel);
    
    voxel* rayCast( // Выполняет трассировку луча через воксельный мир.
        glm::vec3 start, // Начальная точка луча
        glm::vec3 dir, // Направление луча
        float maxDist, // Максимальная дистанция трассировки
        glm::vec3& end, // Точка попадания луча
        glm::vec3& norm, // Нормаль поверхности в точке попадания
        glm::vec3& iend // Координаты вокселя в точке попадания
    );

    void write(unsigned char* dest); // Записывает данные чанков в бинарный поток
    void read(unsigned char* source); // Читает данные чанков из бинарного потока
};

#endif // VOXELS_CHUNKS_H_
