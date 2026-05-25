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
#include <graphics/commons/Model.h>
#include <objects/Entities.h>
#include <logic/scripting/scripting_hud.h>
#include <assets/assets_util.h>
#include <graphics/render/ParticlesRenderer.h>
#include <graphics/render/Emitter.h>
#include <graphics/core/Font.h>
#include <util/listutil.h>
#include <graphics/render/TextNote.h>
#include <graphics/render/TextsRenderer.h>
#include <graphics/render/GuidesRenderer.h>

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
    modelBatch(std::make_unique<ModelBatch>(20'000, engine->getAssets(), level->chunks.get(), &engine->getSettings())),
    particles(std::make_unique<ParticlesRenderer>(*engine->getAssets(), *levelFrontend->getLevel(), &engine->getSettings().graphics)),
    texts(std::make_unique<TextsRenderer>(frustumCulling.get())),
    guides(std::make_unique<GuidesRenderer>())
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
	const Camera& camera, 
	ShaderProgram& shader, 
	bool culling
) {
	auto chunk = level->chunks->getChunks()[index];
    if (chunk == nullptr || !chunk->flags.lighted) return false;

	float distance = glm::distance(
        camera.position,
        glm::vec3(
			(chunk->chunk_x + 0.5f) * CHUNK_WIDTH, 
			camera.position.y, 
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
	shader.uniformMatrix("u_model", model);
	mesh->draw();
    return true;
}

void WorldRenderer::drawChunks(
    Chunks* chunks, const Camera& camera, ShaderProgram& shader
) {
	auto assets = engine->getAssets();
    auto atlas = assets->get<Atlas>("blocks");

    atlas->getTexture()->bind();

    renderer->update();

    int chunksWidth = chunks->getWidth();
    int chunksOffsetX = chunks->getOffsetX();
    int chunksOffsetZ = chunks->getOffsetZ();

	if (indices.size() != chunks->getVolume()) {
        indices.clear();
        for (int i = 0; i < chunks->getVolume(); ++i) {
            indices.push_back(ChunksSortEntry{i, 0});
        }
	}

	float px = camera.position.x / static_cast<float>(CHUNK_WIDTH) - 0.5f;
    float pz = camera.position.z / static_cast<float>(CHUNK_DEPTH) - 0.5f;
    for (auto& index : indices) {
        float x = index.index % chunksWidth + chunksOffsetX - px;
        float z = index.index / chunksWidth + chunksOffsetZ - pz;
        index.d = (x * x + z * z) * 1024;
    }
    util::insertion_sort(indices.begin(), indices.end());

	bool culling = engine->getSettings().graphics.frustumCulling.get();
	if (culling) frustumCulling->update(camera.getProjView());
	chunks->visibleCount = 0;
	if (GLEW_ARB_multi_draw_indirect && false) {
        // TODO: implement Multi Draw Indirect chunks draw
    } else {
        for (size_t i = 0; i < indices.size(); ++i) {
            chunks->visibleCount += drawChunk(indices[i].index, camera, shader, culling);
        }
	}
}

void WorldRenderer::setupWorldShader(
    ShaderProgram& shader,
    const Camera& camera,
    const EngineSettings& settings, 
    float fogFactor
) {
	shader.use();
    shader.uniformMatrix("u_model", glm::mat4(1.0f));
    shader.uniformMatrix("u_proj", camera.getProjection());
    shader.uniformMatrix("u_view", camera.getView());
    shader.uniform1f("u_gamma", settings.graphics.gamma.get());
    shader.uniform1f("u_fogFactor", fogFactor);
    shader.uniform1f("u_fogCurve", settings.graphics.fogCurve.get());
    shader.uniform3f("u_cameraPos", camera.position);
    shader.uniform2f("u_lightDir", skybox->getLightDir());
    shader.uniform1i("u_cubemap", 1);
    shader.uniform1f("u_timer", timer);
    shader.uniform1f("u_dayTime", level->getWorld()->getInfo().daytime);

    auto contentIds = level->content->getIndices();
	{
		auto inventory = player->getInventory();
		ItemStack& stack = inventory->getSlot(player->getChosenSlot());
		auto& choosen_item = contentIds->items.require(stack.getItemId());
		if (!player->isNoclip()) {
			float multiplier = 0.8f;
			shader.uniform3f("u_torchlightColor",
				choosen_item.emission[0] / 15.0f * multiplier,
				choosen_item.emission[1] / 15.0f * multiplier,
				choosen_item.emission[2] / 15.0f * multiplier
			);
		} else {
			shader.uniform3f("u_torchlightColor", 0.0f, 0.0f, 0.0f);
		}

		shader.uniform1f("u_torchlightDistance", 6.0f);
	}
}

void WorldRenderer::renderLevel(
    const DrawContext& ctx,
    const Camera& camera, 
    const EngineSettings& settings,
    float deltaTime,
    bool pause,
    bool hudVisible
) {
    const auto& assets = *engine->getAssets();

    texts->renderTexts(
        *batch3d, ctx, assets, camera, settings, hudVisible, false
    );

    bool culling = engine->getSettings().graphics.frustumCulling.get();

    float fogFactor = 15.0f / ((float)settings.chunks.loadDistance.get() - 2);

    auto& entityShader = assets.require<ShaderProgram>("entity");
    setupWorldShader(entityShader, camera, settings, fogFactor);
    skybox->bind();

    level->entities->render(
        assets,
        *modelBatch,
        culling ? frustumCulling.get() : nullptr,
        deltaTime,
        pause
    );

    particles->render(camera, deltaTime * !pause);
    modelBatch->render();

    auto& shader = assets.require<ShaderProgram>("default");
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
    constexpr auto boxOffset = glm::vec3(0.01);
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
    const Camera& camera,
    ShaderProgram& linesShader,
    const DrawContext& pctx
) {
    linesShader.use();
    linesShader.uniformMatrix("u_projview", camera.getProjView());
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

void WorldRenderer::renderHands(const Camera& camera, const Assets& assets, float deltaTime) {
    auto& entityShader = assets.require<ShaderProgram>("entity");
    auto indices = level->content->getIndices();

    const auto& inventory = player->getInventory();
    int slot = player->getChosenSlot();
    const ItemStack& stack = inventory->getSlot(slot);
    const auto& def = indices->items.require(stack.getItemId());

    Camera hudcam = camera;
    hudcam.far = 100.0f;
    hudcam.setFov(1.2f);
    hudcam.position = {};

    const glm::vec3 itemOffset(0.08f, 0.035f, -0.1);

    static glm::mat4 prevRotation(1.0f);

    const float speed = 24.0f;

    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), itemOffset);
    matrix = glm::scale(matrix, glm::vec3(0.1f));
    glm::mat4 rotation = camera.rotation;
    glm::quat rot0 = glm::quat_cast(prevRotation);
    glm::quat rot1 = glm::quat_cast(rotation);
    glm::quat finalRot = glm::slerp(rot0, rot1, static_cast<float>(deltaTime * speed));
    rotation = glm::mat4_cast(finalRot);
    matrix = rotation * matrix * glm::rotate(glm::mat4(1.0f), -glm::pi<float>() * 0.5f, glm::vec3(0, 1, 0));
    prevRotation = rotation;
    auto offset = -(camera.position - player->getPosition());
    float angle = glm::radians(player->cam.x - 90);
    float cos = glm::cos(angle);
    float sin = glm::sin(angle);

    float newX = offset.x * cos - offset.z * sin;
    float newZ = offset.x * sin + offset.z * cos;
    offset = glm::vec3(newX, offset.y, newZ);
    matrix = matrix * glm::translate(glm::mat4(1.0f), offset);

    modelBatch->setLightsOffset(camera.position);
    modelBatch->draw(
        matrix,
        glm::vec3(1.0f),
        assets.get<model::Model>(def.modelName),
        nullptr
    );
    Window::clearDepth();
    setupWorldShader(entityShader, hudcam, engine->getSettings(), 0.0f);
    skybox->bind();
    modelBatch->render();
    modelBatch->setLightsOffset(glm::vec3());
    skybox->unbind();
}

void WorldRenderer::draw(
    const DrawContext& pctx,
    Camera& camera,
    bool hudVisible,
    bool pause,
    float deltaTime,
    PostProcessing* postProcessing
) {
    timer += deltaTime * !pause;

    auto world = level->getWorld();
    const Viewport& vp = pctx.getViewport();
    camera.aspect = vp.getWidth() / static_cast<float>(vp.getHeight());

    const auto& settings = engine->getSettings();
    const auto& worldInfo = world->getInfo();
    skybox->refresh(pctx, worldInfo.daytime, 1.0f + worldInfo.skyClearness * 2.0f, 4);

    const auto& assets = *engine->getAssets();
    auto& linesShader = assets.require<ShaderProgram>("lines");

    {
        DrawContext wctx = pctx.sub();
        postProcessing->use(wctx);
        Window::clearDepth();
        skybox->draw(pctx, camera, assets, worldInfo.daytime, worldInfo.skyClearness);
        {
            DrawContext ctx = wctx.sub();
            ctx.setDepthTest(true);
            ctx.setCullFace(true);
            renderLevel(ctx, camera, settings, deltaTime, pause, hudVisible);
            if (hudVisible) {
                if (player->debug) {
                    guides->renderDebugLines(
                        ctx, camera, *lineBatch, linesShader, drawChunkBorders
                    );
                }
                renderLines(camera, linesShader, ctx);
                if (!player->isNoclip() && player->currentCamera == player->fpCamera) {
                    renderHands(camera, assets, deltaTime * !pause);
                }
            }
        }

        renderBlockOverlay(wctx, assets);
    }

    auto screenShader = assets.get<ShaderProgram>("screen");
    screenShader->use();
    screenShader->uniform1f("u_timer", timer);
    screenShader->uniform1f("u_dayTime", worldInfo.daytime);
    postProcessing->render(pctx, screenShader);
}

void WorldRenderer::renderBlockOverlay(const DrawContext& wctx, const Assets& assets) {
    int x = std::floor(player->currentCamera->position.x);
    int y = std::floor(player->currentCamera->position.y);
    int z = std::floor(player->currentCamera->position.z);
    auto block = level->chunks->getVoxel(x, y, z);
    if (block && block->id) {
        const auto& def = level->content->getIndices()->blocks.require(block->id);
        if (def.overlayTexture.empty()) return;

        auto textureRegion = util::get_texture_region(
            assets, def.overlayTexture, "blocks:" + TEXTURE_NOTFOUND
        );

        DrawContext ctx = wctx.sub();
        ctx.setDepthTest(false);
        ctx.setCullFace(false);

        auto& shader = assets.require<ShaderProgram>("ui3d");
        shader.use();
        batch3d->begin();
        shader.uniformMatrix("u_projview", glm::mat4(1.0f));
        shader.uniformMatrix("u_apply", glm::mat4(1.0f));
        auto light = level->chunks->getLight(x, y, z);
        float s = LightMap::extract(light, 3) / 15.0f;
        glm::vec4 tint(
            glm::min(1.0f, LightMap::extract(light, 0) / 15.0f + s),
            glm::min(1.0f, LightMap::extract(light, 1) / 15.0f + s),
            glm::min(1.0f, LightMap::extract(light, 2) / 15.0f + s),
            1.0f
        );
        batch3d->texture(textureRegion.texture);
        batch3d->sprite(
            glm::vec3(),
            glm::vec3(0, 1, 0),
            glm::vec3(1, 0, 0),
            2,
            2,
            textureRegion.region,
            tint
        );
        batch3d->flush();
    }
}

void WorldRenderer::clear() {
    renderer->clear();
}
