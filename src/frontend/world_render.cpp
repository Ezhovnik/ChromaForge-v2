#include "world_render.h"

#include <vector>
#include <algorithm>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../window/Window.h"
#include "../window/Camera.h"
#include "../graphics/Mesh.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/LineBatch.h"
#include "../graphics/Batch3D.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../world/LevelEvents.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../objects/player_control.h"
#include "../logger/Logger.h"
#include "../graphics/ChunksRenderer.h"
#include "../math/FrustumCulling.h"
#include "../engine.h"

inline constexpr glm::vec4 CLEAR_COLOR = {0.7f, 0.71f, 0.73f, 1.0f};
inline constexpr float GAMMA_VALUE = 1.6f;
inline constexpr glm::vec3 SKY_LIGHT_COLOR = {1.8f, 1.8f, 1.8f};
inline constexpr glm::vec3 FOG_COLOR = {0.7f, 0.71f, 0.73f};
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

WorldRenderer::WorldRenderer(Engine* engine, Level* level) : engine(engine), level(level) {
	lineBatch = new LineBatch(4096);
	renderer = new ChunksRenderer(level);
    frustumCulling = new Frustum();
    level->events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() {
	delete lineBatch;
	delete renderer;
    delete frustumCulling;
}

// Отрисовывает один чанк
bool WorldRenderer::drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool occlusion){
	std::shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->isLighted()) return false;

    std::shared_ptr<Mesh> mesh = renderer->getOrRender(chunk.get());
	if (mesh == nullptr) return false;

	if (occlusion){
		glm::vec3 min_(chunk->chunk_x * CHUNK_WIDTH, chunk->bottom, chunk->chunk_z * CHUNK_DEPTH);
        glm::vec3 max_(chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH, chunk->top, chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH);

        if (!frustumCulling->isBoxVisible(min_, max_)) return false;
	}

	glm::mat4 model = glm::translate(
        glm::mat4(1.0f), 
        glm::vec3(
            chunk->chunk_x * CHUNK_WIDTH, 
            0.0f, 
            chunk->chunk_z * CHUNK_DEPTH + 1.0f
        ));

	shader->uniformMatrix("u_model", model);

	mesh->draw();

    return true;
}

void WorldRenderer::drawChunks(Chunks* chunks, Camera* camera, ShaderProgram* shader, bool occlusion) {
    std::vector<size_t> indices;
	for (size_t i = 0; i < chunks->volume; i++){
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		indices.push_back(i);
	}

	float px = camera->position.x / (float)CHUNK_WIDTH;
	float pz = camera->position.z / (float)CHUNK_DEPTH;
	std::sort(indices.begin(), indices.end(), [this, chunks, px, pz](size_t i, size_t j) {
		std::shared_ptr<Chunk> a = chunks->chunks[i];
		std::shared_ptr<Chunk> b = chunks->chunks[j];
		return ((a->chunk_x + 0.5f - px)*(a->chunk_x + 0.5f - px) + (a->chunk_z + 0.5f - pz)*(a->chunk_z + 0.5f - pz) >
				(b->chunk_x + 0.5f - px)*(b->chunk_x + 0.5f - px) + (b->chunk_z + 0.5f - pz)*(b->chunk_z + 0.5f - pz));
	});

	if (occlusion) frustumCulling->update(camera->getProjView());
	chunks->visibleCount = 0;
	for (size_t i = 0; i < indices.size(); i++){
		chunks->visibleCount += drawChunk(indices[i], camera, shader, occlusion);
	}
}

void WorldRenderer::draw(const GfxContext& context, Camera* camera, bool occlusion){
    Chunks* chunks = level->chunks;
    EngineSettings& settings = engine->getSettings();
    Assets* assets = engine->getAssets();

    // Загрузка ресурсов с проверкой
	Texture* texture = assets->getTexture("blocks");
    if (texture == nullptr) {
        LOG_CRITICAL("The texture 'bloks' could not be found in the assets");
        throw std::runtime_error("The texture 'bloks' could not be found in the assets");
    }

	ShaderProgram* shader = assets->getShader("default");
    if (shader == nullptr) {
        LOG_CRITICAL("The shader 'default' could not be found in the assets");
        throw std::runtime_error("The shader 'default' could not be found in the assets");
    }
	ShaderProgram* linesShader = assets->getShader("lines");
    if (linesShader == nullptr) {
        LOG_CRITICAL("The shader 'lines' could not be found in the assets");
        throw std::runtime_error("The shader 'lines' could not be found in the assets");
    }

    const Viewport& viewport = context.getViewport();
	int displayWidth = viewport.getWidth();
	int displayHeight = viewport.getHeight();

    {
		GfxContext ctx = context.sub();
		ctx.depthTest(true);
		ctx.cullFace(true);

		EngineSettings& settings = engine->getSettings();

		Window::setBgColor(CLEAR_COLOR);
		Window::clear();
		Window::viewport(0, 0, displayWidth, displayHeight);

		shader->use();
    
		shader->uniformMatrix("u_proj", camera->getProjection());
		shader->uniformMatrix("u_view", camera->getView());
    
		shader->uniform1f("u_gamma", GAMMA_VALUE);
		shader->uniform3f("u_skyLightColor", SKY_LIGHT_COLOR);
    
        float fogFactor = 18.0f / (float)settings.chunks.loadDistance;
		shader->uniform3f("u_fogColor", FOG_COLOR);
		shader->uniform1f("u_fogFactor", fogFactor);
		shader->uniform1f("u_fogCurve", settings.graphics.fogCurve);

		shader->uniform3f("u_cameraPos", camera->position);

		Block* choosen_block = Block::blocks[level->player->choosenBlock].get();
        float multiplier = 0.8f;
        shader->uniform3f("u_torchlightColor",
                choosen_block->emission[0] / MAX_TORCH_LIGHT * multiplier,
                choosen_block->emission[1] / MAX_TORCH_LIGHT * multiplier,
                choosen_block->emission[2] / MAX_TORCH_LIGHT * multiplier);
        shader->uniform1f("u_torchlightDistance", TORCH_LIGHT_DIST);

		Chunks* chunks = level->chunks;
		drawChunks(chunks, camera, shader, occlusion);

		shader->uniformMatrix("u_model", glm::mat4(1.0f));

		if (level->playerController->selectedBlockId != -1 && !level->player->noclip){
            Block* selectedBlock = Block::blocks[level->playerController->selectedBlockId].get();;
            glm::vec3 pos = level->playerController->selectedBlockPosition;

            linesShader->use();
            linesShader->uniformMatrix("u_projview", camera->getProjView());

            lineBatch->setLineWidth(2.0f);
            if (selectedBlock->model == BlockModel::Cube){
                lineBatch->box(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f, 1.005f, 1.005f, 1.005f, 0, 0, 0, 0.5f);
            } else if (selectedBlock->model == BlockModel::X){
                lineBatch->box(pos.x + 0.4f, pos.y + 0.3f, pos.z + 0.4f, 0.805f, 0.805f, 0.805f, 0, 0, 0, 0.5f);
            }

            lineBatch->render();
        }
	}

    if (level->player->debug) {
        float length = 40.0f;

		linesShader->use();
		glm::vec3 tsl(displayWidth / 2, -((int)displayHeight) / 2, 0.0f);
		glm::mat4 model(glm::translate(glm::mat4(1.0f), tsl));
		linesShader->uniformMatrix("u_projview", glm::ortho(
				0.0f, (float)displayWidth, 
				-(float)displayHeight, 0.0f, 
				-length, length) * model * glm::inverse(camera->rotation)
            );

		lineBatch->setLineWidth(4.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 1.0f);
        lineBatch->render();

		lineBatch->setLineWidth(2.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 1.0f);
		lineBatch->render();
	}
}
