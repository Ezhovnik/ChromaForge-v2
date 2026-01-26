#include "VoxelsVolume.h"

VoxelsVolume::VoxelsVolume(int x, int y, int z, int width, int height, int depth) : x(x), y(y), z(z), width(width), height(height), depth(depth) {
	volume = width * height * depth;

    voxels = new voxel[volume];
	for (int i = 0; i < volume; ++i) {
		voxels[i].id = BLOCK_VOID;
	}
	lights = new light_t[volume];
}

VoxelsVolume::VoxelsVolume(int width, int height, int depth) : x(0), y(0), z(0), width(width), height(height), depth(depth) {
	volume = width * height * depth;

    voxels = new voxel[volume];
	for (int i = 0; i < volume; ++i) {
		voxels[i].id = BLOCK_VOID;
	}
	lights = new light_t[volume];
}

VoxelsVolume::~VoxelsVolume() {
	delete[] lights;
	delete[] voxels;
}

void VoxelsVolume::setPosition(int x, int y, int z) {
	this->x = x;
	this->y = y;
	this->z = z;
}
