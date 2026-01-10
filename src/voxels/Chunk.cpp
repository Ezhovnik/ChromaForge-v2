#include "Chunk.h"

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "voxel.h"
#include "../lighting/LightMap.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_z) : chunk_x(chunk_x), chunk_z(chunk_z) {
    voxels = new voxel[CHUNK_VOLUME];

    // Инициализируем воксели
    for(size_t i = 0; i < CHUNK_VOLUME; ++i) {
        voxels[i].id = 0;
    }

	light_map = new LightMap();
    renderData.vertices = nullptr;
}

// Деструктор
Chunk::~Chunk() {
    delete light_map;
    delete[] voxels;
    delete[] renderData.vertices;
}

// Проверяет, является ли чанк пустым (однородным).
bool Chunk::isEmpty() {
    int id = -1;
	for (int i = 0; i < CHUNK_VOLUME; ++i){
		if (voxels[i].id != id){
			if (id != -1) return false;
			else id = voxels[i].id;
		}
	}
	return true;
}

// Создает полную копию текущего чанка.
Chunk* Chunk::clone() const {
	Chunk* other = new Chunk(chunk_x, chunk_z);
	for (int i = 0; i < CHUNK_VOLUME; ++i) {
		other->voxels[i] = voxels[i];
    }
	other->light_map->set(light_map);
	return other;
}

// Увеличивает счетчик ссылок на чанк.
void Chunk::incref(){
	references++;
}

// Уменьшает счётчик ссылок на чанк
// WARN: После вызова этого метода указатель на чанк становится невалидным, если счетчик ссылок достиг нуля.
void Chunk::decref(){
    // Уменьшаем счетчик ссылок и проверяем результат
    // Удаляем чанк, если на него больше никто не ссылается
	if (--references <= 0) delete this;
}
