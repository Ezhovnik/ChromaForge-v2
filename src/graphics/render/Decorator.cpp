#include <graphics/render/Decorator.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <graphics/render/ParticlesRenderer.h>
#include <assets/assets_util.h>
#include <content/Content.h>
#include <voxels/Chunks.h>
#include <voxels/Block.h>
#include <world/Level.h>
#include <window/Camera.h>
#include <math/util.h>

inline constexpr int UPDATE_AREA_DIAMETER = 32;
inline constexpr int UPDATE_BLOCKS = UPDATE_AREA_DIAMETER * UPDATE_AREA_DIAMETER * UPDATE_AREA_DIAMETER;
inline constexpr int ITERATIONS = 512;
inline constexpr int BIG_PRIME = 1791791791;

Decorator::Decorator(
    const Level& level,
    ParticlesRenderer& particles,
    const Assets& assets
) : level(level),
    particles(particles),
    assets(assets) {}

void Decorator::update(
    float delta, const glm::ivec3& areaStart, const glm::ivec3& areaCenter
) {
    int index = currentIndex;
    currentIndex = (currentIndex + BIG_PRIME) % UPDATE_BLOCKS;

    const auto& chunks = *level.chunks;
    const auto& indices = *level.content->getIndices();

    int lx = index % UPDATE_AREA_DIAMETER;
    int lz = (index / UPDATE_AREA_DIAMETER) % UPDATE_AREA_DIAMETER;
    int ly = (index / UPDATE_AREA_DIAMETER / UPDATE_AREA_DIAMETER);
    
    auto pos = areaStart + glm::ivec3(lx, ly, lz);

    if (auto vox = chunks.getVoxel(pos)) {
        const auto& def = indices.blocks.require(vox->id);
        if (def.particles) {
            const auto& found = blockEmitters.find(pos);
            if (found == blockEmitters.end()) {
                auto treg = util::get_texture_region(
                    assets, def.particles->texture, ""
                );
                blockEmitters[pos] = particles.add(std::make_unique<Emitter>(
                    level,
                    glm::vec3{pos.x + 0.5, pos.y + 0.5, pos.z + 0.5},
                    *def.particles,
                    treg.texture,
                    treg.region,
                    -1
                ));
            }
        }
    }
}

void Decorator::update(float delta, const Camera& camera) {
    glm::ivec3 pos = camera.position;
    pos -= glm::ivec3(UPDATE_AREA_DIAMETER / 2);
    for (int i = 0; i < ITERATIONS; ++i) {
        update(delta, pos, camera.position);
    }
    const auto& chunks = *level.chunks;
    const auto& indices = *level.content->getIndices();
    auto iter = blockEmitters.begin();
    while (iter != blockEmitters.end()) {
        auto emitter = particles.getEmitter(iter->second);
        if (emitter == nullptr) {
            iter = blockEmitters.erase(iter);
            continue;
        }

        bool remove = false;
        if (auto vox = chunks.getVoxel(iter->first)) {
            const auto& def = indices.blocks.require(vox->id);
            if (def.particles == nullptr) {
                remove = true;
            }
        } else {
            iter = blockEmitters.erase(iter);
            continue;
        }
        if (util::distance2(iter->first, glm::ivec3(camera.position)) > UPDATE_AREA_DIAMETER * UPDATE_AREA_DIAMETER) {
            remove = true;
        }
        if (remove) {
            emitter->stop();
            iter = blockEmitters.erase(iter);
            continue;
        }
        iter++;
    }
}
