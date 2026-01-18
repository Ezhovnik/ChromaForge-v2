#ifndef SRC_WORLD_RENDERER_H_
#define SRC_WORLD_RENDERER_H_

#include <vector>
#include <algorithm>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class World;
class Level;
class Camera;
class Assets;
class LineBatch;
class Batch3D;
class ChunksRenderer;
class ShaderProgram;
class Texture;
class Framebuffer;

class WorldRenderer {
	Batch3D *batch3D;
	Assets* assets;
	Level* level;
	bool drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool occlusion);
public:
	ChunksRenderer* renderer;
	LineBatch* lineBatch;

	WorldRenderer(Level* level, Assets* assets);
	~WorldRenderer();

	void draw(Camera* camera, bool occlusion);
};


#endif // SRC_WORLD_RENDERER_H_
