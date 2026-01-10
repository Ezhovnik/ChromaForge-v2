#ifndef VOXELS_CHUNKSLOADER_H_
#define VOXELS_CHUNKSLOADER_H_

#include <thread>
#include <atomic>

constexpr int SURROUNDINGS_CNT = 9;

enum LoaderMode {
	OFF, IDLE, LOAD, LIGHTS, RENDER,
};

class Chunk;
class World;

class ChunksLoader final {
private:
	std::thread loaderThread;
	void _thread();
	std::atomic<Chunk*> current {nullptr};
	std::atomic<Chunk**> surroundings {nullptr};
	std::atomic<LoaderMode> state {IDLE};
    World* world;

    void perform(Chunk* chunk, Chunk** closes_passed, LoaderMode mode);
public:
	ChunksLoader(World* world) : loaderThread{}, world(world) {
		loaderThread = std::thread{&ChunksLoader::_thread, this};
	}
	~ChunksLoader(){
		state = OFF;
		loaderThread.join();
	}

	bool isBusy(){
		return current != nullptr;
	}

    void load(Chunk* chunk, Chunk** closes_passed);
	void lights(Chunk* chunk, Chunk** closes_passed);
	void render(Chunk* chunk, Chunk** closes_passed);

	void stop(){
		state = OFF;
	}
};

#endif // VOXELS_CHUNKSLOADER_H_
