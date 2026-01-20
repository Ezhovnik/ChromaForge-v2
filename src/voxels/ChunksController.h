#ifndef VOXELS_CHUNKSCONTROLLER_H_
#define VOXELS_CHUNKSCONTROLLER_H_

#include "../typedefs.h"

class Chunks;
class Lighting;
class WorldFiles;
class Level;

class ChunksController {
private:
	Chunks* chunks;
	Lighting* lighting;
	Level* level;

    int64_t avgDurationMcs = 1000;
public:
	ChunksController(Level* level, Chunks* chunks, Lighting* lighting);
	~ChunksController();

    void update(int64_t maxDuration);
	bool loadVisible(WorldFiles* worldFiles);
};

#endif // VOXELS_CHUNKSCONTROLLER_H_
