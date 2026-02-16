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

class Level;
class Camera;
class LineBatch;
class ChunksRenderer;
class ShaderProgram;
class Texture;
class Frustum;
class Engine;
class Chunks;
class ContentGfxCache;
class Skybox;

class WorldRenderer {
    Engine* engine;
	Level* level;
    Frustum* frustumCulling;
    ChunksRenderer* renderer;
	LineBatch* lineBatch;
	Skybox* skybox;

	bool drawChunkBorders = false;

    bool drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool culling);
	void drawChunks(Chunks* chunks, Camera* camera, ShaderProgram* shader);

	void drawBorders(int start_x, int start_y, int start_z, int end_x, int end_y, int end_z);
public:
	WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache);
	~WorldRenderer();

	void draw(const GfxContext& context, Camera* camera);

	inline bool isChunkBordersOn() {return drawChunkBorders;}
	inline void setChunkBorders(bool flag) {drawChunkBorders = flag;}
};

#endif // FRONTEND_WORLD_RENDERER_H_
