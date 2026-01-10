#include "ChunksLoader.h"

#include <iostream>
#include <chrono>

#include "Chunk.h"
#include "Chunks.h"
#include "WorldGenerator.h"
#include "../lighting/Lighting.h"
#include "../graphics/VoxelRenderer.h"
#include "../world/World.h"
#include "Block.h"
#include "voxel.h"
#include "../declarations.h"
#include "../logger/Logger.h"

void ChunksLoader::_thread(){
	Chunks chunks(3, 3, -1, -1);
	Lighting lighting(&chunks);
    VoxelRenderer renderer;
	while (state != OFF){
		if (current == nullptr){
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		Chunk* chunk = current;
		chunks._setOffset(chunk->chunk_x - 1, chunk->chunk_z - 1);
		for (size_t i = 0; i < SURROUNDINGS_CNT; ++i){
			Chunk* other = surroundings[i];
			if (other) chunks.putChunk(other);
		}

        if (state == LOAD) {
            chunks.putChunk(chunk);
            if(!chunk->isLoaded()) {
                WorldGenerator::generate(chunk->voxels, chunk->chunk_x, chunk->chunk_z, world->seed);
                chunk->setUnsaved(true);
            }

            for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
                if (Block::blocks[chunk->voxels[i].id].get() == nullptr){
                    LOG_WARN("Corruped block detected at {} of chunk {}x {}z", i, chunk->chunk_x, chunk->chunk_z);
					chunk->voxels[i].id = Blocks_id::BEDROCK;
				}
            }

            lighting.preBuildSkyLight(chunk->chunk_x, chunk->chunk_z);
        } else if (state == LIGHTS) {
            lighting.buildSkyLight(chunk->chunk_x, chunk->chunk_z);
            lighting.onChunkLoaded(chunk->chunk_x, chunk->chunk_z);
            chunk->setLighted(true);
        } else if (state == RENDER) {
            chunk->setModified(false);
			size_t size;
			renderer.render(chunk, (const Chunk**)(surroundings.load()), size);

			float* vertices = new float[size];
			for (size_t i = 0; i < size; ++i) {
				vertices[i] = renderer.buffer[i];
            }

			chunk->renderData.vertices = vertices;
			chunk->renderData.size = size;
        }

        chunks.clear(false);
        for (int i = 0; i < SURROUNDINGS_CNT; ++i){
			Chunk* other = surroundings[i];
			if (other) other->decref();
		}
		chunk->setReady(true);
		current = nullptr;
		chunk->decref();
	}
}

void ChunksLoader::perform(Chunk* chunk, Chunk** closes_passed, LoaderMode mode){
	if (isBusy()){
        LOG_WARN("Performing while busy");
		return;
	}

	chunk->incref();
	if (surroundings == nullptr) surroundings = new Chunk*[SURROUNDINGS_CNT];

	for (int i = 0; i < SURROUNDINGS_CNT; ++i){
		Chunk* other = closes_passed[i];
		if (other == nullptr) {
            surroundings[i] = nullptr;
        } else {
			other->incref();
			surroundings[i] = other;
		}
	}

	current = chunk;
    state = mode;
}

void ChunksLoader::load(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, LOAD);
}

void ChunksLoader::lights(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, LIGHTS);
}

void ChunksLoader::render(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, RENDER);
}
