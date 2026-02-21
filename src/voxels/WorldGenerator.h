#ifndef VOXELS_WORLDGENERATOR_H_
#define VOXELS_WORLDGENERATOR_H_

#include "../typedefs.h"

class voxel;
class PseudoRandom;
class Map2D;
class fnl_state;
class Content;

// Класс для генерации воксельного мира
class WorldGenerator {
private:
    blockid_t const idAir;
    blockid_t const idStone;
	blockid_t const idDirt;
	blockid_t const idMoss;
	blockid_t const idSand;
	blockid_t const idWater;
	blockid_t const idLog;
	blockid_t const idLeaves;
	blockid_t const idGrass;
	blockid_t const idPoppy;
    blockid_t const idDandelion;
    blockid_t const idDaisy;
    blockid_t const idMarigold;
	blockid_t const idBedrock;

    blockid_t generate_tree(fnl_state *noise, 
				  PseudoRandom* random, 
				  Map2D& heights,
				  int real_x, 
				  int real_y, 
				  int real_z, 
				  int tileSize);
public:
	WorldGenerator(const Content* content);
	void generate(voxel* voxels, int x, int z, uint64_t seed);
};

#endif // VOXELS_WORLDGENERATOR_H_
