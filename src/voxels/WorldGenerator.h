#pragma once

#include <string>

#include <typedefs.h>

struct voxel;
class PseudoRandom;
class Map2D;
class fnl_state;
class Content;

// Класс для генерации воксельного мира
class WorldGenerator {
protected:
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
public:
	WorldGenerator(const Content* content);

	virtual void generate(voxel* voxels, int x, int z, uint64_t seed) = 0;
};
