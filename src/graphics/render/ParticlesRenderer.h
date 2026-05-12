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
    std::unique_ptr<MainBatch> batch;

    std::unordered_map<uint64_t, std::unique_ptr<Emitter>> emitters;
    uint64_t nextEmitter = 1;

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

    uint64_t add(std::unique_ptr<Emitter> emitter);

    void gc();

    Emitter* getEmitter(uint64_t id) const;

    static size_t visibleParticles;
    static size_t aliveEmitters;
};
