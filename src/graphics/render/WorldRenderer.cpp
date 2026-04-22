#include <graphics/render/WorldRenderer.h>

#include <vector>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <content/Content.h>
#include <window/Window.h>
#include <window/Camera.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Atlas.h>
#include <graphics/core/LineBatch.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <voxels/Block.h>
#include <world/World.h>
#include <world/Level.h>
#include <objects/Player.h>
#include <assets/Assets.h>
#include <logic/PlayerController.h>
#include <debug/Logger.h>
#include <graphics/render/ChunksRenderer.h>
#include <world/LevelEvents.h>
#include <math/FrustumCulling.h>
#include <math/voxmaths.h>
#include <engine.h>
#include <settings.h>
#include <frontend/LevelFrontend.h>
#include <graphics/render/Skybox.h>
#include <constants.h>
#include <items/Item.h>
#include <items/ItemStack.h>
#include <items/Inventory.h>
#include <graphics/core/Batch3D.h>
#include <graphics/core/PostProcessing.h>
#include <graphics/render/ModelBatch.h>
#include <graphics/core/Model.h>
#include <objects/Entities.h>
#include <logic/scripting/scripting_hud.h>

inline constexpr glm::vec3 SKY_LIGHT_COLOR = {0.7f, 0.81f, 1.0f};
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

bool WorldRenderer::drawChunkBorders = false;
bool WorldRenderer::drawEntityHitboxes = false;

WorldRenderer::WorldRenderer(
	Engine* engine,
	LevelFrontend* levelFrontend,
	Player* player
) : engine(engine), 
	level(levelFrontend->getLevel()),
	player(player),
    frustumCulling(std::make_unique<Frustum>()),
    lineBatch(std::make_unique<LineBatch>()),
    modelBatch(std::make_unique<ModelBatch>(20'000, engine->getAssets(), level->chunks.get()))
{
    renderer = std::make_unique<ChunksRenderer>(
        level,
        levelFrontend->getContentGfxCache(),
        &engine->getSettings()
    );
    batch3d = std::make_unique<Batch3D>(4096);

	auto& settings = engine->getSettings();
	auto assets = engine->getAssets();
    skybox = std::make_unique<Skybox>(
        settings.graphics.skyboxResolution.get(), 
        assets->get<ShaderProgram>("skybox_gen")
    );

    level->events->listen(CHUNK_HIDDEN, [this](lvl_event_type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() = default;

// Отрисовывает один чанк
bool WorldRenderer::drawChunk(
	size_t index, 
	Camera* camera, 
	ShaderProgram* shader, 
	bool culling)
{
	std::shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->flags.lighted) return false;

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

		if (!frustumCulling->isBoxVisible(min, max)) return false;
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
	auto assets = engine->getAssets();
    auto atlas = assets->get<Atlas>("blocks");

    atlas->getTexture()->bind();

    renderer->update();
	std::vector<size_t> indices;
	for (size_t i = 0; i < chunks->volume; ++i){
		if (chunks->chunks[i] == nullptr) continue;
		indices.emplace_back(i);
	}

	float px = camera->position.x / static_cast<float>(CHUNK_WIDTH) - 0.5f;
    float pz = camera->position.z / static_cast<float>(CHUNK_DEPTH) - 0.5f;
    std::sort(indices.begin(), indices.end(), [chunks, px, pz](auto i, auto j) {
        const auto a = chunks->chunks[i].get();
        const auto b = chunks->chunks[j].get();
        auto adx = (a->chunk_x - px);
        auto adz = (a->chunk_z - pz);
        auto bdx = (b->chunk_x - px);
        auto bdz = (b->chunk_z - pz);
        return (adx * adx + adz * adz > bdx * bdx + bdz * bdz);
	});

	bool culling = engine->getSettings().graphics.frustumCulling.get();
	if (culling) frustumCulling->update(camera->getProjView());
	chunks->visibleCount = 0;
	for (size_t i = 0; i < indices.size(); ++i){
		chunks->visibleCount += drawChunk(indices[i], camera, shader, culling);
	}
}

void WorldRenderer::setupWorldShader(
    ShaderProgram* shader,
    Camera* camera,
    const EngineSettings& settings, 
    float fogFactor
) {
	shader->use();
    shader->uniformMatrix("u_model", glm::mat4(1.0f));
    shader->uniformMatrix("u_proj", camera->getProjection());
    shader->uniformMatrix("u_view", camera->getView());
    shader->uniform1f("u_gamma", settings.graphics.gamma.get());
    shader->uniform1f("u_fogFactor", fogFactor);
    shader->uniform1f("u_fogCurve", settings.graphics.fogCurve.get());
    shader->uniform3f("u_cameraPos", camera->position);
    shader->uniform1i("u_cubemap", 1);
    shader->uniform1f("u_timer", timer);
    shader->uniform1f("u_dayTime", level->getWorld()->getInfo().daytime);

    auto contentIds = level->content->getIndices();
	{
		auto inventory = player->getInventory();
		ItemStack& stack = inventory->getSlot(player->getChosenSlot());
		auto& choosen_item = contentIds->items.require(stack.getItemId());
		if (!player->isNoclip()) {
			float multiplier = 0.8f;
			shader->uniform3f("u_torchlightColor",
				choosen_item.emission[0] / 15.0f * multiplier,
				choosen_item.emission[1] / 15.0f * multiplier,
				choosen_item.emission[2] / 15.0f * multiplier
			);
		} else {
			shader->uniform3f("u_torchlightColor", 0.0f, 0.0f, 0.0f);
		}

		shader->uniform1f("u_torchlightDistance", 6.0f);
	}
}

void WorldRenderer::renderLevel(
    const DrawContext&,
    Camera* camera, 
    const EngineSettings& settings,
    float deltaTime,
    bool pause
) {
    auto assets = engine->getAssets();

    bool culling = engine->getSettings().graphics.frustumCulling.get();

    float fogFactor = 15.0f / ((float)settings.chunks.loadDistance.get() - 2);

    auto entityShader = assets->get<ShaderProgram>("entity");
    setupWorldShader(entityShader, camera, settings, fogFactor);
    skybox->bind();

    level->entities->render(
        assets,
        *modelBatch,
        culling ? frustumCulling.get() : nullptr,
        deltaTime,
        pause
    );

    modelBatch->render();

    auto shader = assets->get<ShaderProgram>("default");
    setupWorldShader(shader, camera, settings, fogFactor);

    drawChunks(level->chunks.get(), camera, shader);

    if (!pause) {
        scripting::on_frontend_render();
    }

    skybox->unbind();
}

void WorldRenderer::renderBlockSelection() {
    const auto& selection = player->selection;
    auto indices = level->content->getIndices();
    blockid_t id = selection.vox.id;
    auto block = indices->blocks.get(id);
    const glm::ivec3 pos = player->selection.position;
    const glm::vec3 point = selection.hitPosition;
    const glm::vec3 norm = selection.normal;

    const std::vector<AABB>& hitboxes = block->rotatable
        ? block->rt.hitboxes[selection.vox.state.rotation]
        : block->hitboxes;

    lineBatch->setLineWidth(2.0f);
    constexpr auto boxOffset = glm::vec3(0.02);
    constexpr auto boxColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
    for (auto& hitbox: hitboxes) {
        const glm::vec3 center = glm::vec3(pos) + hitbox.center();
        const glm::vec3 size = hitbox.size();
        lineBatch->box(center, size + boxOffset, boxColor);
        if (player->debug) {
            lineBatch->line(point, point + norm * 0.5f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        }
    }
    lineBatch->flush();
}
void WorldRenderer::renderLines(
    Camera* camera,
    ShaderProgram* linesShader,
    const DrawContext& pctx
) {
    linesShader->use();
    linesShader->uniformMatrix("u_projview", camera->getProjView());
    if (player->selection.vox.id != BLOCK_VOID && !player->isNoclip()) {
        renderBlockSelection();
    }
    if (player->debug && drawEntityHitboxes) {
        auto ctx = pctx.sub(lineBatch.get());
        bool culling = engine->getSettings().graphics.frustumCulling.get();
        level->entities->renderDebug(
            *lineBatch, culling ? frustumCulling.get() : nullptr, ctx
        );
    }
}

void WorldRenderer::renderDebugLines(
    const DrawContext& pctx, 
    Camera* camera,
    ShaderProgram* linesShader
) {
    DrawContext ctx = pctx.sub(lineBatch.get());
    const auto& viewport = ctx.getViewport();
    uint displayWidth = viewport.getWidth();
    uint displayHeight = viewport.getHeight();
    ctx.setDepthTest(true);

    linesShader->use();

    if (drawChunkBorders) {
        linesShader->uniformMatrix("u_projview", camera->getProjView());
        glm::vec3 coord = player->camera->position;
        if (coord.x < 0) coord.x--;
        if (coord.z < 0) coord.z--;
        int cx = floordiv(static_cast<int>(coord.x), CHUNK_WIDTH);
        int cz = floordiv(static_cast<int>(coord.z), CHUNK_DEPTH);

        drawBorders(
            cx * CHUNK_WIDTH, 0, cz * CHUNK_DEPTH, 
            (cx + 1) * CHUNK_WIDTH, CHUNK_HEIGHT, (cz + 1) * CHUNK_DEPTH
        );
    }

    float length = 40.0f;
    glm::vec3 tsl(displayWidth / 2, displayHeight / 2, 0.0f);
    glm::mat4 model(glm::translate(glm::mat4(1.0f), tsl));
    linesShader->uniformMatrix("u_projview", glm::ortho(
        0.0f, static_cast<float>(displayWidth), 
        0.0f, static_cast<float>(displayHeight),
        -length, length) * model * glm::inverse(camera->rotation)
    );

    ctx.setDepthTest(false);
    lineBatch->setLineWidth(4.0f);
    lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 1.0f);
    lineBatch->flush();

    ctx.setDepthTest(true);
    lineBatch->setLineWidth(2.0f);
    lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 1.0f);
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
	lineBatch->flush();
}

void WorldRenderer::draw(
    const DrawContext& pctx,
    Camera* camera,
    bool hudVisible,
    bool pause,
    float deltaTime,
    PostProcessing* postProcessing
) {
    timer += deltaTime * !pause;

    auto world = level->getWorld();
    const Viewport& vp = pctx.getViewport();
    camera->aspect = vp.getWidth() / static_cast<float>(vp.getHeight());

    const auto& settings = engine->getSettings();
    const auto& worldInfo = world->getInfo();
    skybox->refresh(pctx, worldInfo.daytime, 1.0f + worldInfo.skyClearness * 2.0f, 4);

    Assets* assets = engine->getAssets();
    ShaderProgram* linesShader = assets->get<ShaderProgram>("lines");

    {
        DrawContext wctx = pctx.sub();
        postProcessing->use(wctx);
        Window::clearDepth();
        skybox->draw(pctx, camera, assets, worldInfo.daytime, worldInfo.skyClearness);
        {
            DrawContext ctx = wctx.sub();
            ctx.setDepthTest(true);
            ctx.setCullFace(true);
            renderLevel(ctx, camera, settings, deltaTime, pause);
            if (hudVisible) {
                renderLines(camera, linesShader, ctx);
            }
        }

        if (hudVisible && player->debug) renderDebugLines(wctx, camera, linesShader);
    }

    auto screenShader = assets->get<ShaderProgram>("screen");
    screenShader->use();
    screenShader->uniform1f("u_timer", timer);
    screenShader->uniform1f("u_dayTime", worldInfo.daytime);
    postProcessing->render(pctx, screenShader);
}
