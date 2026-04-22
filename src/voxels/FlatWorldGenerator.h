#pragma once

#include <typedefs.h>
#include <voxels/WorldGenerator.h>

struct voxel;
class Content;

class FlatWorldGenerator : public WorldGenerator {
public:
	FlatWorldGenerator(const Content* content) : WorldGenerator(content) {}

	void generate(voxel* voxels, int x, int z, uint64_t seed) override;
};
