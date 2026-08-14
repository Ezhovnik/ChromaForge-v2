#pragma once

#include <vector>
#include <string>
#include <memory>

#include <graphics/render/commons.h>
#include <graphics/core/DrawContext.h>
#include <presets/WeatherPreset.h>
#include <window/Camera.h>
#include <util/ObjectsKeeper.h>

class Level;
class LineBatch;
class ChunksRenderer;
class ShaderProgram;
class PostProcessing;
class Frustum;
class Engine;
class LevelFrontend;
class Skybox;
class Batch3D;
class Player;
struct EngineSettings;
class ModelBatch;
class Assets;
class ParticlesRenderer;
class TextsRenderer;
class DebugLinesRenderer;
class BlockWrapsRenderer;
class PrecipitationRenderer;
class HandsRenderer;
class NamedSkeletons;
class Shadows;
class CloudsRenderer;
struct Weather;

class WorldRenderer final : public util::ObjectsKeeper {
public:
    static bool drawChunkBorders;
    static bool drawEntityHitboxes;

    WorldRenderer(Engine& engine, LevelFrontend& levelFrontend, Player& player);
    ~WorldRenderer();

    void update(const Camera& camera, float deltaTime);

    void renderFrame(
        const DrawContext& context,
        Camera& camera,
        bool hudVisible,
        PostProcessing& postProcessing
    );

    void resetCache();

    void setDebug(bool flag);
    void toggleLightsDebug();

    Weather& getWeather();
private:
    Engine& engine;
    const Level& level;
    Player& player;
    const Assets& assets;
    Weather& weather;
    std::unique_ptr<Frustum> frustumCulling;
    std::unique_ptr<LineBatch> lineBatch;
    std::unique_ptr<Batch3D> batch3d;
    std::unique_ptr<ModelBatch> modelBatch;
    std::unique_ptr<ChunksRenderer> chunksRenderer;
    std::unique_ptr<HandsRenderer> hands;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<Shadows> shadowMapping;
    std::unique_ptr<DebugLinesRenderer> debugLines;
    std::unique_ptr<PrecipitationRenderer> precipitation;
    std::unique_ptr<CloudsRenderer> cloudsRenderer;

    float timer = 0.0f;
    bool debug = false;
    bool lightsDebug = false;
    bool gbufferPipeline = false;
    bool dirtySettings = true;

    void renderBlockSelection();
    void renderInWorldLines(
        const Camera& camera,
        ShaderProgram& linesShader,
        const DrawContext& pctx
    );

    void renderBlockOverlay(
        const DrawContext& context
    );

    void setupWorldShader(
        ShaderProgram& shader,
        const Camera& camera,
        const EngineSettings& settings,
        float fogFactor
    );

    void renderOpaque(
        const DrawContext& context, 
        const Camera& camera, 
        const EngineSettings& settings,
        bool hudVisible
    );

    void renderOpaquePass(
        const DrawContext& context,
        Camera& camera,
        bool hudVisible,
        PostProcessing& postProcessing
    );

    void renderWeatherEffects(Camera& camera);

    void renderHandsPass(const DrawContext& pctx, Camera& camera);

    void renderDebugLines(const DrawContext& context, Camera& camera);

    void renderFrameClassic(
        const DrawContext& context, 
        Camera& camera, 
        bool hudVisible,
        PostProcessing& postProcessing
    );

    void renderFrameAdvanced(
        const DrawContext& context, 
        Camera& camera, 
        bool hudVisible,
        PostProcessing& postProcessing
    );

    void refreshSettings();

    float calcFogFactor() const;
public:
    std::unique_ptr<ParticlesRenderer> particles;
    std::unique_ptr<TextsRenderer> texts;
    std::unique_ptr<BlockWrapsRenderer> blockWraps;
    std::unique_ptr<NamedSkeletons> skeletons;
};
