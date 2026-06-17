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
#include <engine/Engine.h>
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
#include <graphics/render/TextNote.h>
#include <graphics/render/TextsRenderer.h>
#include <graphics/render/GuidesRenderer.h>
#include <graphics/render/BlockWrapsRenderer.h>
#include <frontend/ContentGfxCache.h>

inline constexpr glm::vec3 SKY_LIGHT_COLOR = {0.7f, 0.81f, 1.0f};
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

inline constexpr size_t BATCH3D_CAPACITY = 4096;
inline constexpr size_t MODEL_BATCH_CAPACITY = 20'000;

bool WorldRenderer::drawChunkBorders = false;
bool WorldRenderer::drawEntityHitboxes = false;

WorldRenderer::WorldRenderer(
	Engine& engine,
	LevelFrontend& levelFrontend,
	Player& player
) : engine(engine), 
	level(levelFrontend.getLevel()),
	player(player),
    assets(*engine.getAssets()),
    frustumCulling(std::make_unique<Frustum>()),
    lineBatch(std::make_unique<LineBatch>()),
    batch3d(std::make_unique<Batch3D>(BATCH3D_CAPACITY)),
    modelBatch(std::make_unique<ModelBatch>(MODEL_BATCH_CAPACITY, assets, *player.chunks, engine.getSettings())),
    particles(std::make_unique<ParticlesRenderer>(assets, level, *player.chunks, &engine.getSettings().graphics)),
    texts(std::make_unique<TextsRenderer>(*batch3d, assets, *frustumCulling)),
    guides(std::make_unique<GuidesRenderer>()),
    chunks(std::make_unique<ChunksRenderer>(&level, *player.chunks, assets, *frustumCulling, levelFrontend.getContentGfxCache(), engine.getSettings())),
    blockWraps(std::make_unique<BlockWrapsRenderer>(assets, level, *player.chunks))
{
	auto& settings = engine.getSettings();
    level.events->listen(CHUNK_HIDDEN, [this](LevelEventType, Chunk* chunk) {
		chunks->unload(chunk);
	});
	auto assets = engine.getAssets();
    skybox = std::make_unique<Skybox>(
        settings.graphics.skyboxResolution.get(), 
        assets->require<ShaderProgram>("skybox_gen")
    );
}

WorldRenderer::~WorldRenderer() = default;

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
    shader.uniform1f("u_dayTime", level.getWorld()->getInfo().daytime);

    auto contentIds = level.content.getIndices();
	{
		auto inventory = player.getInventory();
		ItemStack& stack = inventory->getSlot(player.getChosenSlot());
		auto& choosen_item = contentIds->items.require(stack.getItemId());
		if (!player.isNoclip()) {
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
    texts->render(ctx, camera, settings, hudVisible, false);

    bool culling = engine.getSettings().graphics.frustumCulling.get();

    float fogFactor = 15.0f / ((float)settings.chunks.loadDistance.get() - 2);

    auto& entityShader = assets.require<ShaderProgram>("entity");
    setupWorldShader(entityShader, camera, settings, fogFactor);
    skybox->bind();

    if (culling) frustumCulling->update(camera.getProjView());

    entityShader.uniform1i("u_alphaClip", true);
    level.entities->render(
        assets,
        *modelBatch,
        culling ? frustumCulling.get() : nullptr,
        deltaTime,
        pause
    );

    modelBatch->render();
    particles->render(camera, deltaTime * !pause);

    auto& shader = assets.require<ShaderProgram>("default");
    auto& linesShader = assets.require<ShaderProgram>("lines");
    setupWorldShader(shader, camera, settings, fogFactor);

    chunks->drawChunks(camera, shader);
    blockWraps->draw(ctx, player);

    if (hudVisible) renderLines(camera, linesShader, ctx);

    shader.use();
    chunks->drawSortedMeshes(camera, shader);

    if (!pause) {
        scripting::on_frontend_render();
    }

    skybox->unbind();
}

void WorldRenderer::renderBlockSelection() {
    const auto& selection = player.selection;
    auto indices = level.content.getIndices();
    blockid_t id = selection.vox.id;
    auto block = indices->blocks.get(id);
    const glm::ivec3 pos = player.selection.position;
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
        if (debug) {
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
    if (player.selection.vox.id != BLOCK_VOID && !player.isNoclip()) {
        renderBlockSelection();
    }
    if (debug && drawEntityHitboxes) {
        auto ctx = pctx.sub(lineBatch.get());
        bool culling = engine.getSettings().graphics.frustumCulling.get();
        level.entities->renderDebug(
            *lineBatch, culling ? frustumCulling.get() : nullptr, ctx
        );
    }
}

void WorldRenderer::renderHands(const Camera& camera, float deltaTime) {
    auto& entityShader = assets.require<ShaderProgram>("entity");
    auto indices = level.content.getIndices();

    const auto& inventory = player.getInventory();
    int slot = player.getChosenSlot();
    const ItemStack& stack = inventory->getSlot(slot);
    const auto& def = indices->items.require(stack.getItemId());

    Camera hudcam = camera;
    hudcam.far = 10.0f;
    hudcam.setFov(0.9f);
    hudcam.position = {};

    const glm::vec3 itemOffset(0.06f, 0.035f, -0.1);

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
    auto offset = -(camera.position - player.getPosition());
    float angle = glm::radians(player.rotation.x - 90);
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
    setupWorldShader(entityShader, hudcam, engine.getSettings(), 0.0f);
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

    auto world = level.getWorld();
    const Viewport& vp = pctx.getViewport();
    camera.aspect = vp.getWidth() / static_cast<float>(vp.getHeight());

    const auto& settings = engine.getSettings();
    const auto& worldInfo = world->getInfo();
    skybox->refresh(pctx, worldInfo.daytime, 1.0f + worldInfo.skyClearness * 2.0f, 4);

    const auto& assets = *engine.getAssets();
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
                if (debug) {
                    guides->renderDebugLines(
                        ctx, camera, *lineBatch, linesShader, drawChunkBorders
                    );
                }
                if (!player.isNoclip() && player.currentCamera == player.fpCamera) {
                    renderHands(camera, deltaTime * !pause);
                }
            }
        }
        {
            DrawContext ctx = wctx.sub();
            texts->render(ctx, camera, settings, hudVisible, true);
        }
        renderBlockOverlay(wctx);
    }

    auto screenShader = assets.get<ShaderProgram>("screen");
    screenShader->use();
    screenShader->uniform1f("u_timer", timer);
    screenShader->uniform1f("u_dayTime", worldInfo.daytime);
    postProcessing->render(pctx, screenShader);
}

void WorldRenderer::renderBlockOverlay(const DrawContext& wctx) {
    int x = std::floor(player.currentCamera->position.x);
    int y = std::floor(player.currentCamera->position.y);
    int z = std::floor(player.currentCamera->position.z);
    auto block = player.chunks->getVoxel(x, y, z);
    if (block && block->id) {
        const auto& def = level.content.getIndices()->blocks.require(block->id);
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
        auto light = player.chunks->getLight(x, y, z);
        float s = Lightmap::extract(light, 3) / 15.0f;
        glm::vec4 tint(
            glm::min(1.0f, Lightmap::extract(light, 0) / 15.0f + s),
            glm::min(1.0f, Lightmap::extract(light, 1) / 15.0f + s),
            glm::min(1.0f, Lightmap::extract(light, 2) / 15.0f + s),
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
    chunks->clear();
}

void WorldRenderer::setDebug(bool flag) {
    debug = flag;
}
