#pragma once

#include <typedefs.h>
#include <constants.h>
#include <voxels/voxel.h>

class VoxelsVolume {
private:
	int x, y, z;
	int width, height, depth;
    int volume;
	std::unique_ptr<voxel[]> voxels;
	std::unique_ptr<light_t[]> lights;
public:
	VoxelsVolume(int width, int height, int depth);
	VoxelsVolume(int x, int y, int z, int width, int height, int depth);
	virtual ~VoxelsVolume();

	void setPosition(int x, int y, int z);

	int getX() const {
		return x;
	}

	int getY() const {
		return y;
	}

	int getZ() const {
		return z;
	}

	int getW() const {
		return width;
	}

	int getH() const {
		return height;
	}

	int getD() const {
		return depth;
	}

	voxel* getVoxels() const {
		return voxels.get();
	}

	light_t* getLights() const {
		return lights.get();
	}

	inline blockid_t pickBlockId(int bx, int by, int bz) const {
		if (bx < x || by < y || bz < z || bx >= x + width || by >= y + height || bz >= z + depth) {
			return BLOCK_VOID;
		}
		return voxels[vox_index(bx - x, by - y, bz - z, width, depth)].id;
	}

	inline light_t pickLight(int bx, int by, int bz) const {
		if (bx < x || by < y || bz < z || bx >= x + width || by >= y + height || bz >= z + depth) {
			return 0;
		}
		return lights[vox_index(bx - x, by - y, bz - z, width, depth)];
	}
};
