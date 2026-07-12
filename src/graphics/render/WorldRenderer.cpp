#include <graphics/render/WorldRenderer.h>

#include <vector>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <string>

#include <content/Content.h>
#include <window/Window.h>
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
#include <graphics/render/PrecipitationRenderer.h>
#include <world/Weather.h>
#include <graphics/core/ShadowMap.h>
#include <graphics/core/GBuffer.h>

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
    modelBatch(
        std::make_unique<ModelBatch>(
            MODEL_BATCH_CAPACITY,
            assets,
            *player.chunks,
            engine.getSettings()
        )
    ),
    guides(std::make_unique<GuidesRenderer>()),
    chunks(
        std::make_unique<ChunksRenderer>(
            &level,
            *player.chunks,
            assets,
            *frustumCulling,
            levelFrontend.getContentGfxCache(),
            engine.getSettings()
        )
    ),
    particles(
        std::make_unique<ParticlesRenderer>(
            assets,
            level,
            *player.chunks,
            &engine.getSettings().graphics
        )
    ),
    texts(
        std::make_unique<TextsRenderer>(
            *batch3d, assets, *frustumCulling
        )
    ),
    blockWraps(
        std::make_unique<BlockWrapsRenderer>(
            assets,
            level,
            *player.chunks
        )
    ),
    precipitation(
        std::make_unique<PrecipitationRenderer>(
            assets,
            level,
            *player.chunks,
            &engine.getSettings().graphics
        )
    )
{
	auto& settings = engine.getSettings();
    level.events->listen(LevelEventType::CHUNK_HIDDEN, [this](LevelEventType, Chunk* chunk) {
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
    shader.uniform1i("u_debugLights", lightsDebug);
    shader.uniform1i("u_debugNormals", false);
    shader.uniform1f("u_weatherFogOpacity", weather.fogOpacity());
    shader.uniform1f("u_weatherFogDencity", weather.fogDencity());
    shader.uniform1f("u_weatherFogCurve", weather.fogCurve());
    shader.uniform3f("u_cameraPos", camera.position);
    shader.uniform2f("u_lightDir", skybox->getLightDir());
    shader.uniform1i("u_skybox", 1);
    shader.uniform1f("u_timer", timer);
    shader.uniform1f("u_dayTime", level.getWorld()->getInfo().daytime);
    shader.uniform1i("u_enableShadows", shadows);

    if (shadows) {
        shader.uniformMatrix("u_shadowsMatrix[0]", shadowCamera.getProjView());
        shader.uniformMatrix("u_shadowsMatrix[1]", wideShadowCamera.getProjView());
        shader.uniform3f("u_sunDir", shadowCamera.front);
        shader.uniform1i("u_shadowsRes", shadowMap->getResolution());

        glActiveTexture(GL_TEXTURE4);
        shader.uniform1i("u_shadows[0]", 4);
        glBindTexture(GL_TEXTURE_2D, shadowMap->getDepthMap());

        glActiveTexture(GL_TEXTURE5);
        shader.uniform1i("u_shadows[1]", 5);
        glBindTexture(GL_TEXTURE_2D, wideShadowMap->getDepthMap());

        glActiveTexture(GL_TEXTURE0);
    }

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
    entityShader.uniform1f("u_opacity", 1.0f);
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

    setupWorldShader(entityShader, camera, settings, fogFactor);

    std::array<const WeatherPreset*, 2> weatherInstances {&weather.a, &weather.b};
    for (const auto& weather : weatherInstances) {
        float maxIntensity = weather->fall.maxIntensity;
        float zero = weather->fall.minOpacity;
        float one = weather->fall.maxOpacity;
        float t = (weather->intensity * (one - zero)) * maxIntensity + zero;
        entityShader.uniform1i("u_alphaClip", weather->fall.opaque);
        entityShader.uniform1f("u_opacity", weather->fall.opaque ? t * t : t);
        if (weather->intensity > 1.e-3f && !weather->fall.texture.empty()) {
            precipitation->render(camera, pause ? 0.0f : deltaTime, *weather);
        }
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
    glm::vec3 cameraRotation = player.getRotation();
    auto offset = -(camera.position - player.getPosition());
    float angle = glm::radians(cameraRotation.x - 90);
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
    display::clearDepth();
    setupWorldShader(entityShader, hudcam, engine.getSettings(), 0.0f);
    skybox->bind();
    modelBatch->render();
    modelBatch->setLightsOffset(glm::vec3());
    skybox->unbind();
}

void WorldRenderer::generateShadowsMap(
    const Camera& camera,
    const DrawContext& pctx,
    ShadowMap& shadowMap,
    Camera& shadowCamera,
    float scale
) {
    auto& shadowsShader = assets.require<ShaderProgram>("shadows");

    auto world = level.getWorld();
    const auto& worldInfo = world->getInfo();

    const auto& settings = engine.getSettings();
    int resolution = shadowMap.getResolution();
    float shadowMapScale = 0.2f / (1 << glm::max(0LL, settings.graphics.shadowsQuality.get())) * scale;
    float shadowMapSize = resolution * shadowMapScale;
    glm::vec3 basePos = glm::floor(camera.position);
    shadowCamera = Camera(basePos, shadowMapSize);
    shadowCamera.near = 0.1f;
    shadowCamera.far = 800.0f;
    shadowCamera.perspective = false;
    shadowCamera.setAspectRatio(1.0f);

    float sunAngle = glm::radians(fmod(90.0f - worldInfo.daytime * 360.0f, 180.0f));
    shadowCamera.rotate(
        sunAngle,
        glm::radians(-45.0f),
        glm::radians(-0.0f)
    );
    shadowCamera.updateVectors();
    shadowCamera.position -= shadowCamera.front * 200.0f;
    shadowCamera.position += shadowCamera.up * 10.0f;
    shadowCamera.position += camera.front * 100.0f;

    auto view = shadowCamera.getView();

    auto currentPos = shadowCamera.position;
    auto min = view * glm::vec4(currentPos - (shadowCamera.right + shadowCamera.up) * (shadowMapSize * 0.5f), 1.0f);
    auto max = view * glm::vec4(currentPos + (shadowCamera.right + shadowCamera.up) * (shadowMapSize * 0.5f), 1.0f);

    shadowCamera.setProjection(glm::ortho(min.x, max.x, min.y, max.y, 0.1f, 800.0f));
    {
        frustumCulling->update(shadowCamera.getProjView());
        auto sctx = pctx.sub();
        sctx.setDepthTest(true);
        sctx.setCullFace(true);
        sctx.setViewport({resolution, resolution});
        shadowMap.bind();
        setupWorldShader(shadowsShader, shadowCamera, settings, 0.0f);
        chunks->drawChunksShadowsPass(shadowCamera, shadowsShader);
        shadowMap.unbind();
    }
}

void WorldRenderer::draw(
    const DrawContext& pctx,
    Camera& camera,
    bool hudVisible,
    bool pause,
    float uiDelta,
    PostProcessing& postProcessing
) {
    float deltaTime = uiDelta * !pause;
    timer += deltaTime;
    weather.update(deltaTime);

    auto world = level.getWorld();
    const auto& vp = pctx.getViewport();
    camera.setAspectRatio(vp.x / static_cast<float>(vp.y));

    const auto& settings = engine.getSettings();

    gbufferPipeline = settings.graphics.advancedRender.get();
    int shadowsQuality = settings.graphics.shadowsQuality.get();
    int resolution = 1024 << shadowsQuality;
    if (shadowsQuality > 0 && !shadows) {
        shadowMap = std::make_unique<ShadowMap>(resolution);
        wideShadowMap = std::make_unique<ShadowMap>(resolution);
        shadows = true;
    } else if (shadowsQuality == 0) {
        shadowMap.reset();
        wideShadowMap.reset();
        shadows = false;
    }
    if (shadows && shadowMap->getResolution() != resolution) {
        shadowMap = std::make_unique<ShadowMap>(resolution);
        wideShadowMap = std::make_unique<ShadowMap>(resolution);
    }

    const auto& worldInfo = world->getInfo();

    float clouds = weather.b.clouds * glm::sqrt(weather.t) + weather.a.clouds * glm::sqrt(1.0f - weather.t);
    clouds = glm::max(worldInfo.skyClearness, clouds);
    float mie = 1.0f + glm::max(worldInfo.skyClearness, clouds * 0.5f) * 2.0f;
    skybox->refresh(pctx, worldInfo.daytime, mie, 4);

    chunks->update();

    static int frameid = 0;
    if (shadows && frameid % 3 == 0) {
        if (frameid % 2 == 0) {
            generateShadowsMap(
                camera,
                pctx,
                *shadowMap,
                shadowCamera,
                1.0f
            );
        } else {
            generateShadowsMap(
                camera,
                pctx,
                *wideShadowMap,
                wideShadowCamera,
                4.0f
            );
        }
    }
    frameid++;

    auto& linesShader = assets.require<ShaderProgram>("lines");
    {
        DrawContext wctx = pctx.sub();
        postProcessing.use(wctx, gbufferPipeline);
        display::clearDepth();
        skybox->draw(pctx, camera, assets, worldInfo.daytime, clouds);
        {
            DrawContext ctx = wctx.sub();
            ctx.setDepthTest(true);
            ctx.setCullFace(true);
            renderLevel(ctx, camera, settings, uiDelta, pause, hudVisible);
            if (hudVisible) {
                if (debug) {
                    guides->renderDebugLines(
                        ctx, camera, *lineBatch, linesShader, drawChunkBorders
                    );
                }
                if (player.currentCamera == player.fpCamera) {
                    renderHands(camera, deltaTime);
                }
            }
        }
        {
            DrawContext ctx = wctx.sub();
            texts->render(ctx, camera, settings, hudVisible, true);
        }
    }

    postProcessing.render(
        pctx,
        assets,
        timer,
        camera,
        shadows ? shadowMap->getDepthMap() : 0
    );
    renderBlockOverlay(pctx);

    glActiveTexture(GL_TEXTURE0);
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

void WorldRenderer::toggleLightsDebug() {
    lightsDebug = !lightsDebug;
}

Weather& WorldRenderer::getWeather() {
    return weather;
}
