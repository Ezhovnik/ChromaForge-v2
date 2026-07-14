#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <graphics/core/DrawContext.h>
#include <presets/WeatherPreset.h>
#include <world/Weather.h>
#include <window/Camera.h>

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
class GuidesRenderer;
class BlockWrapsRenderer;
class PrecipitationRenderer;
class HandsRenderer;
class NamedSkeletons;
class ShadowMap;
class GBuffer;

struct CompileTimeShaderSettings {
    bool advancedRender = false;
    bool shadows = false;
	bool ssao = false;
};

class WorldRenderer {
private:
    Engine& engine;
    const Level& level;
    Player& player;
    const Assets& assets;
    std::unique_ptr<Frustum> frustumCulling;
    std::unique_ptr<LineBatch> lineBatch;
    std::unique_ptr<Batch3D> batch3d;
    std::unique_ptr<ModelBatch> modelBatch;
    std::unique_ptr<GuidesRenderer> guides;
    std::unique_ptr<ChunksRenderer> chunks;
	std::unique_ptr<HandsRenderer> hands;
    std::unique_ptr<Skybox> skybox;
	std::unique_ptr<ShadowMap> shadowMap;
	std::unique_ptr<ShadowMap> wideShadowMap;
    Weather weather {};

	Camera shadowCamera;
	Camera wideShadowCamera;

	float timer = 0.0f;

	bool debug = false;
	bool lightsDebug = false;

	bool gbufferPipeline = false;
    bool shadows = false;

	CompileTimeShaderSettings prevCTShaderSettings {};

	void renderBlockSelection();
	void renderLines(
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

	void generateShadowsMap(
        const Camera& camera,
        const DrawContext& pctx,
        ShadowMap& shadowMap,
        Camera& shadowCamera,
        float scale
    );
public:
	std::unique_ptr<ParticlesRenderer> particles;
    std::unique_ptr<TextsRenderer> texts;
    std::unique_ptr<BlockWrapsRenderer> blockWraps;
    std::unique_ptr<PrecipitationRenderer> precipitation;
	std::unique_ptr<NamedSkeletons> skeletons;

	WorldRenderer(Engine& engine, LevelFrontend& levelFrontend, Player& player);
	~WorldRenderer();

	void draw(
		const DrawContext& context,
		Camera& camera,
		bool hudVisible,
		bool pause,
		float deltaTime,
		PostProcessing& postProcessing
	);

	void renderLevel(
        const DrawContext& context,
        const Camera& camera,
        const EngineSettings& settings,
		float deltaTime,
		bool pause,
		bool hudVisible
    );

	void clear();

	void setDebug(bool flag);
	void toggleLightsDebug();

	Weather& getWeather();

	static bool drawChunkBorders;
	static bool drawEntityHitboxes;
};
