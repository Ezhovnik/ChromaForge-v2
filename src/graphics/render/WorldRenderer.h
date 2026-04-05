#ifndef FRONTEND_WORLD_RENDERER_H_
#define FRONTEND_WORLD_RENDERER_H_

#include <vector>
#include <algorithm>
#include <string>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../../graphics/core/DrawContext.h"

class Camera;
class Level;
class LineBatch;
class ChunksRenderer;
class ShaderProgram;
class PostProcessing;
class Frustum;
class Engine;
class Chunks;
class LevelFrontend;
class Skybox;
class Batch3D;
class Player;
struct EngineSettings;
class ModelBatch;

namespace model {
    struct Model;
}

class WorldRenderer {
private:
    Engine* engine;
	Player* player;
	Level* level;
    std::unique_ptr<Frustum> frustumCulling;
    std::unique_ptr<ChunksRenderer> renderer;
	std::unique_ptr<LineBatch> lineBatch;
	std::unique_ptr<Skybox> skybox;
	std::unique_ptr<Batch3D> batch3d;
	std::unique_ptr<ModelBatch> modelBatch;

    bool drawChunk(
		size_t index, 
		Camera* camera, 
		ShaderProgram* shader, 
		bool culling
	);
	void drawChunks(
		Chunks* chunks, 
		Camera* camera, 
		ShaderProgram* shader
	);
	void renderBlockSelection();
	void renderDebugLines(
        const DrawContext& context, 
        Camera* camera, 
        ShaderProgram* linesShader
    );
	void renderLines(Camera* camera, ShaderProgram* linesShader);

	void drawBorders(int start_x, int start_y, int start_z, int end_x, int end_y, int end_z);
public:
	WorldRenderer(Engine* engine, LevelFrontend* levelFrontend, Player* player);
	~WorldRenderer();

	void draw(
		const DrawContext& context, 
		Camera* camera, 
		bool hudVisible,
		PostProcessing* postProcessing
	);

	void renderLevel(
        const DrawContext& context, 
        Camera* camera, 
        const EngineSettings& settings
    );

	static bool drawChunkBorders;
	static bool drawEntityHitboxes;
};

#endif // FRONTEND_WORLD_RENDERER_H_
