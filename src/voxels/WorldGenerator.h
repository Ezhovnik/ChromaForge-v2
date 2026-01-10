#ifndef VOXELS_WORLDGENERATOR_H_
#define VOXELS_WORLDGENERATOR_H_

class voxel;

// Класс для генерации воксельного мира
class WorldGenerator {
public:
	static void generate(voxel* voxels, int chunk_x, int chunk_z, int seed); // Генерирует воксельные данные для чанка
};

#endif // VOXELS_WORLDGENERATOR_H_
