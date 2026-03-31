#include "VoxelsVolume.h"

VoxelsVolume::VoxelsVolume(
	int x, int y, int z,
	int width, int height, int depth
) : x(x), y(y), z(z),
	width(width), height(height), depth(depth),
	volume(width * height * depth),
	voxels(std::make_unique<voxel[]>(volume)),
    lights(std::make_unique<light_t[]>(volume))
{
    for (int i = 0; i < volume; ++i) {
        voxels[i].id = BLOCK_VOID;
    }
}

VoxelsVolume::VoxelsVolume(int width, int height, int depth) : VoxelsVolume(0, 0, 0, width, height, depth) {
}

VoxelsVolume::~VoxelsVolume() {
}

void VoxelsVolume::setPosition(int x, int y, int z) {
	this->x = x;
	this->y = y;
	this->z = z;
}
