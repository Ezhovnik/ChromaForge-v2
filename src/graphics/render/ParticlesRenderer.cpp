#include <graphics/render/ParticlesRenderer.h>

#include <set>

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
#include <assets/assets_util.h>

size_t ParticlesRenderer::visibleParticles = 0;
size_t ParticlesRenderer::aliveEmitters = 0;

ParticlesRenderer::ParticlesRenderer(
    const Assets& assets,
    const Level& level,
    const Chunks& chunks,
    const GraphicsSettings* settings
) : batch(std::make_unique<MainBatch>(4096)),
    level(level),
    chunks(chunks),
    assets(assets),
    settings(settings) {}

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
            auto& preset = particle.emitter->preset;

            if (!preset.frames.empty()) {
                float time = preset.lifetime - particle.lifetime;
                int framesCount = preset.frames.size();
                int frameid = time / preset.lifetime * framesCount;
                int frameid2 = glm::min(
                    (time + deltaTime) / preset.lifetime * framesCount,
                    framesCount - 1.0f
                );
                if (frameid2 != frameid) {
                    auto tregion = util::get_texture_region(
                        assets, preset.frames.at(frameid2), ""
                    );
                    if (tregion.texture == texture) {
                        particle.region = tregion.region;
                    }
                }
            }

            update_particle(particle, deltaTime, chunks);

            glm::vec4 light(1, 1, 1, 0);
            if (preset.lighting) {
                light = MainBatch::sampleLight(
                    particle.position, chunks, backlight
                );
                light *= 0.9f + (particle.random % 100) * 0.001f;
            }
            float scale = 1.0f + ((particle.random ^ 2628172) % 1000) * 0.001f * preset.sizeSpread;
            batch->quad(
                particle.position,
                right,
                preset.globalUpVector ? glm::vec3(0, 1, 0) : up,
                preset.size * scale,
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
        auto& emitter = *iter->second;
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

void ParticlesRenderer::gc() {
    std::set<Emitter*> usedEmitters;
    for (const auto& [_, vec] : particles) {
        for (const auto& particle : vec) {
            usedEmitters.insert(particle.emitter);
        }
    }
    auto iter = emitters.begin();
    while (iter != emitters.end()) {
        auto emitter = iter->second.get();
        if (usedEmitters.find(emitter) == usedEmitters.end()) {
            iter = emitters.erase(iter);
        } else {
            iter++;
        }
    }
}

Emitter* ParticlesRenderer::getEmitter(uint64_t id) const {
    const auto& found = emitters.find(id);
    if (found == emitters.end()) {
        return nullptr;
    }
    return found->second.get();
}

uint64_t ParticlesRenderer::add(std::unique_ptr<Emitter> emitter) {
    uint64_t uid = nextEmitter++;
    emitters[uid] = std::move(emitter);
    return uid;
}
