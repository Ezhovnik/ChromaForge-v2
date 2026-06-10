#include <world/LevelEvents.h>

#include <voxels/Chunk.h>

void LevelEvents::listen(LevelEventType type, const ChunkEventFunc& func) {
	auto& callbacks = chunk_callbacks[type];
	callbacks.push_back(func);
}

void LevelEvents::trigger(LevelEventType type, Chunk* chunk) {
	const auto& callbacks = chunk_callbacks[type];
	for (const ChunkEventFunc& func : callbacks) {
		func(type, chunk);
	}
}
