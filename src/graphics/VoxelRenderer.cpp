#include "VoxelRenderer.h"

#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"

const int VERTEX_SIZE = (3 + 2 + 1);
const int CHUNK_ATTRS[] = {3, 2, 1, 0};

#define CDIV(X, A) (((X) < 0) ? ((X) / (A) - 1) : ((X) / (A)))
#define LOCAL_NEG(X, SIZE) (((X) < 0) ? ((SIZE) + (X)) : (X))
#define LOCAL(X, SIZE) ((X) >= (SIZE) ? ((X) - (SIZE)) : LOCAL_NEG(X, SIZE))
#define IS_CHUNK(X, Y, Z) (GET_CHUNK(X, Y, Z) != nullptr)
#define GET_CHUNK(X, Y, Z) (closes[((CDIV(Y, CHUNK_HEIGHT) + 1) * 3 + CDIV(Z, CHUNK_DEPTH) + 1) * 3 + CDIV(X, CHUNK_WIDTH) + 1])

#define VOXEL(X, Y, Z) (GET_CHUNK(X, Y, Z)->voxels[(LOCAL(Y, CHUNK_HEIGHT) * CHUNK_DEPTH + LOCAL(Z, CHUNK_DEPTH)) * CHUNK_WIDTH + LOCAL(X, CHUNK_WIDTH)])
#define IS_BLOCKED(X, Y, Z) ((!IS_CHUNK(X, Y, Z)) || VOXEL(X, Y, Z).id)

#define VERTEX(INDEX, X, Y, Z, U, V, L) buffer[INDEX + 0] = (X);\
								  buffer[INDEX + 1] = (Y);\
								  buffer[INDEX + 2] = (Z);\
								  buffer[INDEX + 3] = (U);\
								  buffer[INDEX + 4] = (V);\
								  buffer[INDEX + 5] = (L);\
								  INDEX += VERTEX_SIZE;



VoxelRenderer::VoxelRenderer(size_t capacity) : capacity(capacity) {
	buffer = new float[capacity * VERTEX_SIZE * 6];
}

VoxelRenderer::~VoxelRenderer(){
	delete[] buffer;
}

Mesh* VoxelRenderer::render(Chunk* chunk, const Chunk** closes){
	size_t index = 0;

	for (int y = 0; y < CHUNK_HEIGHT; y++){
		for (int z = 0; z < CHUNK_DEPTH; z++){
			for (int x = 0; x < CHUNK_WIDTH; x++){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				uint id = vox.id;

				if (!id){
					continue;
				}

				float light;

				float uvsize = 1.0f / 16.0f;
				float u = (id % 16) * uvsize;
				float v = 1.0f - (1 + id / 16) * uvsize;

				if (!IS_BLOCKED(x, y + 1, z)){
					light = 1.0f;
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);

					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u, v, light);
				}
				if (!IS_BLOCKED(x, y - 1, z)){
					light = 0.75f;
					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u, v + uvsize, light);

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
				}

				if (!IS_BLOCKED(x + 1, y, z)){
					light = 0.95f;
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v + uvsize, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);

					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u, v, light);
				}
				if (!IS_BLOCKED(x - 1, y, z)){
					light = 0.85f;
					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u, v + uvsize, light);

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u, v, light);
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v, light);
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
				}

				if (!IS_BLOCKED(x, y, z + 1)){
					light = 0.9f;
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u, v, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u, v + uvsize, light);

					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u, v, light);
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u + uvsize, v, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u + uvsize, v + uvsize, light);
				}
				if (!IS_BLOCKED(x, y, z - 1)){
					light = 0.8f;
					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u + uvsize, v + uvsize, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u, v + uvsize, light);

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u + uvsize, v, light);
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u, v + uvsize, light);
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u, v, light);
				}
			}
		}
	}
	return new Mesh(buffer, index / VERTEX_SIZE, CHUNK_ATTRS);
}
