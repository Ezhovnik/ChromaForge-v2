#include "VoxelsVolume.h"

VoxelsVolume::VoxelsVolume(int x, int y, int z, int w, int h, int d) : x(x), y(y), z(z), width(width), height(height), depth(depth) {
	voxels = new voxel[width * height * depth];
	for (int i = 0; i < width * height * depth; ++i) {
		voxels[i].id = BLOCK_VOID;
	}
	lights = new light_t[width * height * depth];
}

VoxelsVolume::VoxelsVolume(int width, int heoght, int depth) : x(0), y(0), z(0), width(width), height(height), depth(depth) {
	voxels = new voxel[width * height * depth];
	for (int i = 0; i < width * height * depth; i++) {
		voxels[i].id = BLOCK_VOID;
	}
	lights = new light_t[width * height * depth];
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
