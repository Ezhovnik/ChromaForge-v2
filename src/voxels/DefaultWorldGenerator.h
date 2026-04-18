#ifndef VOXELS_DEFAULTWORLDGENERATOR_H_
#define VOXELS_DEFAULTWORLDGENERATOR_H_

#include <string>

#include "typedefs.h"
#include "WorldGenerator.h"

struct voxel;
class Content;

class DefaultWorldGenerator : public WorldGenerator {
private:
    blockid_t generate_tree(
		fnl_state *noise, 
		PseudoRandom* random, 
		Map2D& heights,
		int real_x, 
		int real_y, 
		int real_z, 
		int tileSize
	);
public:
	DefaultWorldGenerator(const Content* content) : WorldGenerator(content) {}

	void generate(voxel* voxels, int x, int z, uint64_t seed) override;
};

#endif // VOXELS_DEFAULTWORLDGENERATOR_H_
