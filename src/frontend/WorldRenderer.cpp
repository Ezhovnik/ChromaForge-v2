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
#include "graphics/ChunksRenderer.h"
#include "../world/LevelEvents.h"
#include "../math/FrustumCulling.h"
#include "../math/voxmaths.h"
#include "../engine.h"
#include "../settings.h"
#include "LevelFrontend.h"
#include "graphics/Skybox.h"
#include "../constants.h"
#include "../items/Item.h"
#include "../items/ItemStack.h"
#include "../items/Inventory.h"
#include "../graphics/Batch3D.h"

inline constexpr glm::vec3 SKY_LIGHT_COLOR = {0.7f, 0.81f, 1.0f};
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

float WorldRenderer::skyClearness = 0.0f;
bool WorldRenderer::drawChunkBorders = false;

WorldRenderer::WorldRenderer(
	Engine* engine,
	LevelFrontend* levelFrontend,
	Player* player
) : engine(engine), 
	level(levelFrontend->getLevel()),
	player(player)
{
	frustumCulling = std::make_unique<Frustum>();
    lineBatch = std::make_unique<LineBatch>();
    renderer = std::make_unique<ChunksRenderer>(
        level,
        levelFrontend->getContentGfxCache(),
        engine->getSettings()
    );
    batch3d = std::make_unique<Batch3D>(4096);

	auto& settings = engine->getSettings();
	auto assets = engine->getAssets();
    skybox = std::make_unique<Skybox>(
        settings.graphics.skyboxResolution, 
        assets->getShader("skybox_gen")
    );

    level->events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() {
}

// Отрисовывает один чанк
bool WorldRenderer::drawChunk(
	size_t index, 
	Camera* camera, 
	ShaderProgram* shader, 
	bool culling)
{
	std::shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->isLighted()) return false;

	float distance = glm::distance(
        camera->position,
        glm::vec3(
			(chunk->chunk_x + 0.5f) * CHUNK_WIDTH, 
			camera->position.y, 
			(chunk->chunk_z + 0.5f) * CHUNK_DEPTH
		)
    );
	auto mesh = renderer->getOrRender(chunk, distance < CHUNK_WIDTH * 1.5f);
	if (mesh == nullptr) return true;

	if (culling){
		glm::vec3 min(
			chunk->chunk_x * CHUNK_WIDTH, 
			chunk->bottom, 
			chunk->chunk_z * CHUNK_DEPTH
		);
		glm::vec3 max(
			chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH, 
			chunk->top, 
			chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH
		);

		if (!frustumCulling->IsBoxVisible(min, max)) return false;
	}
	glm::vec3 coord = glm::vec3(
		chunk->chunk_x * CHUNK_WIDTH + 0.5f,
		0.5f,
		chunk->chunk_z * CHUNK_DEPTH + 0.5f
	);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);
	shader->uniformMatrix("u_model", model);
	mesh->draw();
    return true;
}

void WorldRenderer::drawChunks(Chunks* chunks, Camera* camera, ShaderProgram* shader) {
	renderer->update();
	std::vector<size_t> indices;
	for (size_t i = 0; i < chunks->volume; ++i){
		if (chunks->chunks[i] == nullptr) continue;
		indices.push_back(i);
	}

	float px = camera->position.x / (float)CHUNK_WIDTH;
	float pz = camera->position.z / (float)CHUNK_DEPTH;
	std::sort(indices.begin(), indices.end(), [chunks, px, pz](size_t i, size_t j) {
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

void WorldRenderer::draw(const GfxContext& parent_context, Camera* camera, bool hudVisible) {
	Window::clearDepth();
	EngineSettings& settings = engine->getSettings();
	skybox->refresh(parent_context, level->world->daytime, 1.0f + skyClearness * 2.0f, 4);

    const Content* content = level->content;
	const ContentIndices* contentIds = content->getIndices();

    Assets* assets = engine->getAssets();

    // Загрузка ресурсов с проверкой
	Atlas* atlas = assets->getAtlas("blocks");

	ShaderProgram* shader = assets->getShader("default");

    const Viewport& viewport = parent_context.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

	skybox->draw(parent_context, camera, assets, level->getWorld()->daytime, skyClearness);

	ShaderProgram* linesShader = assets->getShader("lines");
    {
		GfxContext context = parent_context.sub();
		context.depthTest(true);
		context.cullFace(true);

		shader->use();
		shader->uniformMatrix("u_proj", camera->getProjection());
		shader->uniformMatrix("u_view", camera->getView());
		shader->uniform1f("u_gamma", settings.graphics.gamma);

        float fogFactor = 15.0f / ((float)settings.chunks.loadDistance - 2);
		shader->uniform1f("u_fogFactor", fogFactor);
		shader->uniform1f("u_fogCurve", settings.graphics.fogCurve);

		shader->uniform3f("u_cameraPos", camera->position);

		shader->uniform1i("u_cubemap", 1);

		{
            auto inventory = player->getInventory();
            ItemStack& stack = inventory->getSlot(player->getChosenSlot());
            Item* chosen_item = contentIds->getItemDef(stack.getItemId());
			assert(chosen_item != nullptr);
			if (!player->noclip) {
				float multiplier = 0.5f;
				shader->uniform3f("u_torchlightColor",
					chosen_item->emission[0] / 15.0f * multiplier,
					chosen_item->emission[1] / 15.0f * multiplier,
					chosen_item->emission[2] / 15.0f * multiplier
				);
			} else {
				shader->uniform3f("u_torchlightColor", 0.0f, 0.0f, 0.0f);
			}

			shader->uniform1f("u_torchlightDistance", 6.0f);
		}

		skybox->bind();
		atlas->getTexture()->bind();

		drawChunks(level->chunks.get(), camera, shader);

		shader->uniformMatrix("u_model", glm::mat4(1.0f));

		if (PlayerController::selectedBlockId != -1 && !player->noclip && hudVisible){
			blockid_t id = PlayerController::selectedBlockId;
			Block* block = contentIds->getBlockDef(id);
			assert(block != nullptr);
			const glm::vec3 pos = PlayerController::selectedBlockPosition;
			const glm::vec3 point = PlayerController::selectedPointPosition;
			const glm::vec3 norm = PlayerController::selectedBlockNormal;

			std::vector<AABB>& hitboxes = block->rotatable ? block->rt.hitboxes[PlayerController::selectedBlockStates] : block->hitboxes;

			linesShader->use();
            linesShader->uniformMatrix("u_projview", camera->getProjView());
            lineBatch->setLineWidth(2.0f);
            for (auto& hitbox: hitboxes) {
                const glm::vec3 center = pos + hitbox.center();
                const glm::vec3 size = hitbox.size();
                lineBatch->box(center, size + glm::vec3(0.02), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
                if (player->debug) lineBatch->line(point, point + norm * 0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
            }
			lineBatch->render();
		}
		skybox->unbind();
	}

    if (hudVisible && player->debug) {
		GfxContext ctx = parent_context.sub();
		ctx.depthTest(true);

		linesShader->use();
		if (drawChunkBorders) {
			linesShader->uniformMatrix("u_projview", camera->getProjView());
			glm::vec3 coord = player->camera->position;
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
