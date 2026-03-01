#ifndef FRONTEND_WORLD_RENDERER_H_
#define FRONTEND_WORLD_RENDERER_H_

#include <vector>
#include <algorithm>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../graphics/GfxContext.h"

class Camera;
class Level;
class LineBatch;
class ChunksRenderer;
class ShaderProgram;
class Texture;
class Frustum;
class Engine;
class Chunks;
class LevelFrontend;
class Skybox;

class WorldRenderer {
    Engine* engine;
	Level* level;
    Frustum* frustumCulling;
    ChunksRenderer* renderer;
	LineBatch* lineBatch;
	Skybox* skybox;

    bool drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool culling);
	void drawChunks(Chunks* chunks, Camera* camera, ShaderProgram* shader);

	void drawBorders(int start_x, int start_y, int start_z, int end_x, int end_y, int end_z);
public:
	WorldRenderer(Engine* engine, LevelFrontend* levelFrontend);
	~WorldRenderer();

	void draw(const GfxContext& context, Camera* camera, bool hudVisible);

	static float skyClearness;
	static bool drawChunkBorders;
};

#endif // FRONTEND_WORLD_RENDERER_H_
