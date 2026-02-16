#include "WorldRenderer.h"

#include <vector>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../content/Content.h"
#include "../window/Window.h"
#include "../window/Camera.h"
#include "../graphics/Mesh.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"
#include "../graphics/LineBatch.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../logic/PlayerController.h"
#include "../logger/Logger.h"
#include "../graphics/ChunksRenderer.h"
#include "../world/LevelEvents.h"
#include "../math/FrustumCulling.h"
#include "../math/voxmaths.h"
#include "../engine.h"
#include "../settings.h"
#include "ContentGfxCache.h"
#include "graphics/Skybox.h"

inline constexpr float GAMMA_VALUE = 1.6f;
inline constexpr glm::vec3 SKY_LIGHT_COLOR = {0.7f, 0.81f, 1.0f};
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

WorldRenderer::WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache) : engine(engine), level(level), frustumCulling(new Frustum()), lineBatch(new LineBatch()), renderer(new ChunksRenderer(level, cache, engine->getSettings())) {
	Assets* assets = engine->getAssets();
	skybox = new Skybox(64, assets->getShader("skybox_gen"));

    level->events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() {
	delete skybox;
	delete lineBatch;
	delete renderer;
    delete frustumCulling;
}

// Отрисовывает один чанк
bool WorldRenderer::drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool culling){
	std::shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->isLighted()) return false;

	std::shared_ptr<Mesh> mesh = renderer->getOrRender(chunk.get());
	if (mesh == nullptr) return true;

	if (culling){
		glm::vec3 min(chunk->chunk_x * CHUNK_WIDTH, chunk->bottom, chunk->chunk_z * CHUNK_DEPTH);
		glm::vec3 max(chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH, chunk->top, chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH);

		if (!frustumCulling->IsBoxVisible(min, max)) return false;
	}

	glm::vec3 coord = glm::vec3(chunk->chunk_x * CHUNK_WIDTH, 0.0f, chunk->chunk_z * CHUNK_DEPTH + 1);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);

	shader->uniformMatrix("u_model", model);

	mesh->draw();

    return true;
}

void WorldRenderer::drawChunks(Chunks* chunks, Camera* camera, ShaderProgram* shader) {
	std::vector<size_t> indices;
	for (size_t i = 0; i < chunks->volume; ++i){
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		indices.push_back(i);
	}

	float px = camera->position.x / (float)CHUNK_WIDTH;
	float pz = camera->position.z / (float)CHUNK_DEPTH;
	std::sort(indices.begin(), indices.end(), [this, chunks, px, pz](size_t i, size_t j) {
		std::shared_ptr<Chunk> a = chunks->chunks[i];
		std::shared_ptr<Chunk> b = chunks->chunks[j];
		return ((a->chunk_x + 0.5f - px) * (a->chunk_x + 0.5f - px) + (a->chunk_z + 0.5f - pz) * (a->chunk_z + 0.5f - pz) >
				(b->chunk_x + 0.5f - px) * (b->chunk_x + 0.5f - px) + (b->chunk_z + 0.5f - pz) * (b->chunk_z + 0.5f - pz));
	});

	bool culling = engine->getSettings().graphics.frustumCulling;
	if (culling) frustumCulling->update(camera->getProjView());
	chunks->visibleCount = 0;
	for (size_t i = 0; i < indices.size(); ++i){
		chunks->visibleCount += drawChunk(indices[i], camera, shader, culling);
	}
}

void WorldRenderer::draw(const GfxContext& parent_context, Camera* camera) {
	EngineSettings& settings = engine->getSettings();
	skybox->refresh(level->world->daytime, fmax(1.0f, 18.0f / settings.chunks.loadDistance), 4);

    const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

    Assets* assets = engine->getAssets();

    // Загрузка ресурсов с проверкой
	Atlas* atlas = assets->getAtlas("blocks");

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

    const Viewport& viewport = parent_context.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

	Window::clearDepth();
	Window::viewport(0, 0, width, height);

	ShaderProgram* backShader = assets->getShader("background");
	backShader->use();
	backShader->uniformMatrix("u_view", camera->getView(false));
	backShader->uniform1f("u_zoom", camera->zoom);
	backShader->uniform1f("u_ar", (float)Window::width/(float)Window::height);
	skybox->draw(backShader);

    {
		GfxContext context = parent_context.sub();
		context.depthTest(true);
		context.cullFace(true);

		shader->use();
		shader->uniformMatrix("u_proj", camera->getProjection());
		shader->uniformMatrix("u_view", camera->getView());
		shader->uniform1f("u_gamma", GAMMA_VALUE);

        float fogFactor = 18.0f / (float)settings.chunks.loadDistance;
		shader->uniform1f("u_fogFactor", fogFactor);
		shader->uniform1f("u_fogCurve", settings.graphics.fogCurve);

		shader->uniform3f("u_cameraPos", camera->position);

		shader->uniform1i("u_cubemap", 1);

		{
			blockid_t id = level->player->choosenBlock;
			Block* choosen_block = contentIds->getBlockDef(id);
			assert(choosen_block != nullptr);
			if (!level->player->noclip) {
				float multiplier = 0.5f;
				shader->uniform3f("u_torchlightColor",
					choosen_block->emission[0] / 15.0f * multiplier,
					choosen_block->emission[1] / 15.0f * multiplier,
					choosen_block->emission[2] / 15.0f * multiplier
				);
			} else {
				shader->uniform3f("u_torchlightColor", 0.0f, 0.0f, 0.0f);
			}
			
			shader->uniform1f("u_torchlightDistance", 6.0f);
		}

		skybox->bind();
		atlas->getTexture()->bind();

		drawChunks(level->chunks, camera, shader);

		shader->uniformMatrix("u_model", glm::mat4(1.0f));

		if (PlayerController::selectedBlockId != -1 && !level->player->noclip){
			blockid_t id = PlayerController::selectedBlockId;
			Block* block = contentIds->getBlockDef(id);
			assert(block != nullptr);
			const glm::vec3 pos = PlayerController::selectedBlockPosition;

			const AABB& hitbox = block->hitbox;
			const glm::vec3 center = pos + hitbox.center();
			const glm::vec3 size = hitbox.size();
			linesShader->use();
			linesShader->uniformMatrix("u_projview", camera->getProjView());
			lineBatch->setLineWidth(2.0f);
			lineBatch->box(center, size + glm::vec3(0.02), glm::vec4(0.f, 0.f, 0.f, 0.5f));
			lineBatch->render();
		}
		skybox->unbind();
	}

    if (level->player->debug) {
		GfxContext ctx = parent_context.sub();
		ctx.depthTest(true);

		linesShader->use();
		if (drawChunkBorders) {
			linesShader->uniformMatrix("u_projview", camera->getProjView());
			glm::vec3 coord = level->player->camera->position;
			if (coord.x < 0) coord.x--;
			if (coord.z < 0) coord.z--;
			int chunk_x = floordiv((int)coord.x, CHUNK_WIDTH);
			int chunk_z = floordiv((int)coord.z, CHUNK_DEPTH);
			
			drawBorders(chunk_x * CHUNK_WIDTH, 0, chunk_z * CHUNK_DEPTH, (chunk_x + 1) * CHUNK_WIDTH, CHUNK_HEIGHT, (chunk_z + 1) * CHUNK_DEPTH);
		}

        float length = 40.0f;

		glm::mat4 model(1.0f);
        glm::vec3 tsl = glm::vec3(width / 2, height / 2, 0.0f);
		model = glm::translate(model, tsl);
		linesShader->uniformMatrix("u_projview", glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -length, length) * model * glm::inverse(camera->rotation));

		ctx.depthTest(false);
		lineBatch->setLineWidth(4.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->render();

		ctx.depthTest(true);
		lineBatch->setLineWidth(2.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 1.0f);
		lineBatch->render();
	}
}

void WorldRenderer::drawBorders(int start_x, int start_y, int start_z, int end_x, int end_y, int end_z) {
	int ww = end_x - start_x;
	int dd = end_z - start_z;
	{
		lineBatch->line(start_x, start_y, start_z, start_x, end_y, start_z, 0.8f, 0, 0.8f, 1);
		lineBatch->line(start_x, start_y, end_z, start_x, end_y, end_z, 0.8f, 0, 0.8f, 1);
		lineBatch->line(end_x, start_y, start_z, end_x, end_y, start_z, 0.8f, 0, 0.8f, 1);
		lineBatch->line(end_x, start_y, end_z, end_x, end_y, end_z, 0.8f, 0, 0.8f, 1);
	}
	for (int i = 2; i < ww; i += 2) {
		lineBatch->line(start_x + i, start_y, start_z, start_x + i, end_y, start_z, 0, 0, 0.8f, 1);
		lineBatch->line(start_x + i, start_y, end_z, start_x + i, end_y, end_z, 0, 0, 0.8f, 1);
	}
	for (int i = 2; i < dd; i += 2) {
		lineBatch->line(start_x, start_y, start_z + i, start_x, end_y, start_z + i, 0.8f, 0, 0, 1);
		lineBatch->line(end_x, start_y, start_z + i, end_x, end_y, start_z + i, 0.8f, 0, 0, 1);
	}
	for (int i = start_y; i < end_y; i += 2){
		lineBatch->line(start_x, i, start_z, start_x, i, end_z, 0, 0.8f, 0, 1);
		lineBatch->line(start_x, i, end_z, end_x, i, end_z, 0, 0.8f, 0, 1);
		lineBatch->line(end_x, i, end_z, end_x, i, start_z, 0, 0.8f, 0, 1);
		lineBatch->line(end_x, i, start_z, start_x, i, start_z, 0, 0.8f, 0, 1);
	}
	lineBatch->render();
}
