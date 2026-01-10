#ifndef LIGHTING_LIGHTSOLVER_H_
#define LIGHTING_LIGHTSOLVER_H_

#include <queue>

#include "../typedefs.h"

class Chunks;

struct lightentry {
	int x;
	int y;
	int z;
	ubyte light;
};

class LightSolver {
	std::queue<lightentry> add_queue;
	std::queue<lightentry> rem_queue;
	Chunks* chunks;
	int channel;
public:
	LightSolver(Chunks* chunks, int channel);

	void add(int x, int y, int z);
	void add(int x, int y, int z, int bright);
	void remove(int x, int y, int z);
	void solve();
};

#endif // LIGHTING_LIGHTSOLVER_H_
