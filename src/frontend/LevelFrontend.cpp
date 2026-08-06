#include <frontend/LevelFrontend.h>

#include <world/Level.h>
#include <assets/Assets.h>
#include <graphics/core/Atlas.h>
#include <graphics/render/BlocksPreview.h>
#include <frontend/ContentGfxCache.h>
#include <content/Content.h>
#include <logic/LevelController.h>
#include <logic/PlayerController.h>
#include <voxels/Block.h>
#include <audio/audio.h>
#include <objects/Player.h>
#include <settings.h>
#include <engine/Engine.h>
#include <physics/Hitbox.h>
#include <voxels/Chunks.h>

LevelFrontend::LevelFrontend(
    Engine& engine,
    PlayerController& playerController,
    LevelController& controller,
    const EngineSettings& settings
) : level(*controller.getLevel()),
    controller(controller),
    assets(*engine.getAssets()), 
    contentCache(std::make_unique<ContentGfxCache>(level.content, assets, settings.graphics))
{
    assets.store(
        BlocksPreview::build(
            engine.getWindow(),
            *contentCache,
            *engine.getAssets(),
            *level.content.getIndices()
        ),
        "block-previews"
    );

    auto& currentPlayer = playerController.getPlayer();
    auto& assets = this->assets;
    auto& level = this->level;

    playerController.setFootstepCallback(
        [&level, &currentPlayer, &assets](const Hitbox& hitbox) {
        const BlockMaterial* material = nullptr;
        if (hitbox.groundMaterial.empty()) {
            const auto& pos = hitbox.position;
            const auto& half = hitbox.getHalfSize();

            auto& blockIndices = level.content.getIndices()->blocks;
            for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
                for (int offsetX = -1; offsetX <= 1; ++offsetX) {
                    int x = std::floor(pos.x + half.x * offsetX);
                    int y = std::floor(pos.y - half.y * 1.1f);
                    int z = std::floor(pos.z + half.z * offsetZ);
                    auto vox = currentPlayer.chunks->getVoxel(x, y, z);
                    if (vox) {
                        auto& def = blockIndices.require(vox->id);
                        if (!def.obstacle) {
                            continue;
                        }
                        material = level.content.findBlockMaterial(def.material);
                        break;
                    }
                }
            }
        } else {
            material = level.content.findBlockMaterial(hitbox.groundMaterial);
        }
        if (material == nullptr) {
            return;
        }

        auto sound = assets.get<audio::Sound>(material->stepsSound);
        glm::vec3 pos {};
        auto soundsCamera = currentPlayer.currentCamera.get();
        if (currentPlayer.isCurrentCameraBuiltin()) {
            soundsCamera = currentPlayer.fpCamera.get();
        }
        bool relative = soundsCamera == currentPlayer.fpCamera.get();
        if (!relative) {
            pos = currentPlayer.getPosition();
        }
        audio::play(
            sound, 
            pos, 
            relative, 
            0.333f, 
            1.0f + (rand() % 6 - 3) * 0.05f, 
            false,
            audio::Priority::Low,
            audio::get_channel_index("regular")
        );
    });

    controller.getBlocksController()->listenBlockInteraction(
        [&level, &assets]
        (auto player, const auto& pos, const auto& def, BlockInteraction type) {
            auto material = level.content.findBlockMaterial(def.material);
            if (material == nullptr) return;

            if (type != BlockInteraction::Step) {
                audio::Sound* sound = nullptr;
                switch (type) {
                    case BlockInteraction::Placing:
                        sound = assets.get<audio::Sound>(material->placeSound);
                        break;
                    case BlockInteraction::Destruction:
                        sound = assets.get<audio::Sound>(material->breakSound);
                        break; 
                    default:
                        break;   
                }
                audio::play(
                    sound, 
                    glm::vec3(pos.x, pos.y, pos.z) + 0.5f, 
                    false, 
                    1.0f,
                    1.0f + (rand() % 6 - 3) * 0.05f, 
                    false,
                    audio::Priority::Normal,
                    audio::get_channel_index("regular")
                );
            }
        }
    );
}

LevelFrontend::~LevelFrontend() = default;

Level& LevelFrontend::getLevel() {
    return level;
}

const ContentGfxCache& LevelFrontend::getContentGfxCache() const {
    return *contentCache;
}

ContentGfxCache& LevelFrontend::getContentGfxCache() {
    return *contentCache;
}

LevelController& LevelFrontend::getController() const {
    return controller;
}
