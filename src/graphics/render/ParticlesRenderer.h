#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include <graphics/render/Emitter.h>

class Texture;
class Assets;
class Camera;
class MainBatch;
class Level;
struct GraphicsSettings;

class ParticlesRenderer {
private:
    const Level& level;

    const Assets& assets;

    const GraphicsSettings* settings;

    std::unordered_map<const Texture*, std::vector<Particle>> particles;
    std::vector<std::unique_ptr<Emitter>> emitters;
    std::unique_ptr<MainBatch> batch;

    void renderParticles(const Camera& camera, float deltaTime);
public:
    ParticlesRenderer(
        const Assets& assets,
        const Level& level,
        const GraphicsSettings* settings
    );
    ~ParticlesRenderer();

    void render(
        const Camera& camera,
        float deltaTime
    );

    void add(std::unique_ptr<Emitter> emitter);

    static size_t visibleParticles;
    static size_t aliveEmitters;
};
