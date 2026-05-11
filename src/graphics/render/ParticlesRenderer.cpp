#include <graphics/render/ParticlesRenderer.h>

#include <assets/Assets.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/render/MainBatch.h>
#include <window/Camera.h>
#include <assets/assets_util.h>
#include <core_content_defs.h>

size_t ParticlesRenderer::visibleParticles = 0;
size_t ParticlesRenderer::aliveEmitters = 0;

ParticlesRenderer::ParticlesRenderer(const Assets& assets) : batch(std::make_unique<MainBatch>(1024)) {
    auto region = util::get_texture_region(assets, "blocks:moss_block", "");
    Emitter emitter(
        glm::vec3(0, 100, 0),
        Particle {
            nullptr,
            glm::vec3(),
            glm::vec3(),
            5.0f,
            region.region
        },
        region.texture,
        0.001f,
        1000
    );
    emitters.push_back(std::make_unique<Emitter>(emitter));
}

ParticlesRenderer::~ParticlesRenderer() = default;

void ParticlesRenderer::renderParticles(const Camera& camera, float deltaTime) {
    const auto& right = camera.right;
    const auto& up = camera.up;

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

            particle.position += particle.velocity * deltaTime;

            batch->quad(
                particle.position,
                right,
                up,
                glm::vec2(0.3f),
                glm::vec4(1),
                glm::vec3(1.0f),
                particle.region
            );

            particle.lifetime -= deltaTime;
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
        emitter.update(deltaTime, *vec);

        iter++;
    }
}
