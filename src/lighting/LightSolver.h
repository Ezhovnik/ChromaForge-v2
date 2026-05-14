#pragma once

#include <queue>

#include <typedefs.h>

class Chunks;
class ContentIndices;
class Block;

struct lightentry {
	int x;
	int y;
	int z;
	ubyte light;
};

class LightSolver {
	std::queue<lightentry> add_queue;
	std::queue<lightentry> rem_queue;
    const Block* const* blockDefs;
	Chunks* chunks;
	int channel;
public:
	LightSolver(const ContentIndices* contentIds, Chunks* chunks, int channel);

	void add(int x, int y, int z);
	void add(int x, int y, int z, int bright);
	void remove(int x, int y, int z);
	void solve();
};
