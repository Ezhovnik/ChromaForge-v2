#include <graphics/render/ParticlesRenderer.h>

#include <assets/Assets.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/render/MainBatch.h>
#include <window/Camera.h>
#include <assets/assets_util.h>
#include <core_content_defs.h>
#include <world/Level.h>
#include <voxels/Chunks.h>
#include <settings.h>

size_t ParticlesRenderer::visibleParticles = 0;
size_t ParticlesRenderer::aliveEmitters = 0;

ParticlesRenderer::ParticlesRenderer(
    const Assets& assets,
    const Level& level,
    const GraphicsSettings* settings
) : batch(std::make_unique<MainBatch>(1024)),
    level(level),
    settings(settings)
{
    auto region = util::get_texture_region(assets, "blocks:moss_block", "");
    emitters.push_back(
        std::make_unique<Emitter>(
            level,
            glm::vec3(0, 80, 0),
            Particle {
                nullptr,
                0,
                glm::vec3(),
                glm::vec3(),
                5.0f,
                region.region
            },
            region.texture,
            0.002f,
            -1
        )
    );
}

ParticlesRenderer::~ParticlesRenderer() = default;

static inline void update_particle(
    Particle& particle, float deltaTime, const Chunks& chunks
) {
    const auto& preset = particle.emitter->preset;
    auto& pos = particle.position;
    auto& vel = particle.velocity;

    vel += deltaTime * preset.acceleration;
    if (preset.collision && chunks.isObstacleAt(pos + vel * deltaTime)) {
        vel *= 0.0f;
    }
    pos += vel * deltaTime;
    particle.lifetime -= deltaTime;
}

void ParticlesRenderer::renderParticles(const Camera& camera, float deltaTime) {
    const auto& right = camera.right;
    const auto& up = camera.up;

    const auto& chunks = *level.chunks;
    bool backlight = settings->backlight.get();

    std::vector<const Texture*> unusedTextures;

    for (auto& [texture, vec] : particles) {
        if (vec.empty()) {
            unusedTextures.push_back(texture);
            continue;
        }
        batch->setTexture(texture);

        visibleParticles += vec.size();

        auto iter = vec.begin();
        while (iter != vec.end()) {
            auto& particle = *iter;

            update_particle(particle, deltaTime, chunks);

            glm::vec4 light(1, 1, 1, 0);
            if (particle.emitter->preset.lighting) {
                light = MainBatch::sampleLight(
                    particle.position, chunks, backlight
                );
                light *= 0.8f + (particle.random % 200) * 0.001f;
            }

            batch->quad(
                particle.position,
                right,
                up,
                glm::vec2(0.3f),
                light,
                glm::vec3(1.0f),
                particle.region
            );

            if (particle.lifetime <= 0.0f) {
                iter = vec.erase(iter);
            } else {
                iter++;
            }
        }
    }
    batch->flush();
    for (const auto& texture : unusedTextures) {
        particles.erase(texture);
    }
}

void ParticlesRenderer::render(const Camera& camera, float deltaTime) {
    batch->begin();

    aliveEmitters = emitters.size();
    visibleParticles = 0;

    renderParticles(camera, deltaTime);

    auto iter = emitters.begin();
    while (iter != emitters.end()) {
        auto& emitter = **iter;
        auto texture = emitter.getTexture();
        const auto& found = particles.find(texture);
        std::vector<Particle>* vec;
        if (found == particles.end()) {
            if (emitter.isDead()) {
                iter = emitters.erase(iter);
                continue;
            }
            vec = &particles[texture];
        } else {
            vec = &found->second;
        }
        emitter.update(deltaTime, camera.position, *vec);

        iter++;
    }
}
