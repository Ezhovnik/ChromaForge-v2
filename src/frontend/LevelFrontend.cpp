#include "LevelFrontend.h"

#include <world/Level.h>
#include "assets/Assets.h"
#include "graphics/core/Atlas.h"
#include "graphics/render/BlocksPreview.h"
#include "ContentGfxCache.h"
#include "content/Content.h"
#include "logic/LevelController.h"
#include "logic/PlayerController.h"
#include <voxels/Block.h>
#include "audio/audio.h"
#include "objects/Player.h"

LevelFrontend::LevelFrontend(
    Player* currentPlayer,
    LevelController* controller, 
    Assets* assets
) : controller(controller),
    level(controller->getLevel()), 
    assets(assets), 
    contentCache(std::make_unique<ContentGfxCache>(level->content, assets))
{
    assets->store(
        BlocksPreview::build(contentCache.get(), assets, level->content),
        "block-previews"
    );

    controller->getBlocksController()->listenBlockInteraction(
        [=](Player* player, glm::ivec3 pos, const Block& def, BlockInteraction type) {
            auto material = level->content->findBlockMaterial(def.material);
            if (material == nullptr) return;

            if (type == BlockInteraction::Step) {
                auto sound = assets->get<audio::Sound>(material->stepsSound);
                glm::vec3 pos {};
                auto soundsCamera = currentPlayer->currentCamera.get();
                if (soundsCamera == currentPlayer->spCamera.get() || soundsCamera == currentPlayer->tpCamera.get()) {
                    soundsCamera = currentPlayer->camera.get();
                }
                bool relative = player == currentPlayer && soundsCamera == currentPlayer->camera.get();
                if (!relative) {
                    pos = player->getPosition();
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
            } else {
                audio::Sound* sound = nullptr;
                switch (type) {
                    case BlockInteraction::Placing:
                        sound = assets->get<audio::Sound>(material->placeSound);
                        break;
                    case BlockInteraction::Destruction:
                        sound = assets->get<audio::Sound>(material->breakSound);
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

Level* LevelFrontend::getLevel() const {
    return level;
}

Assets* LevelFrontend::getAssets() const {
    return assets;
}

ContentGfxCache* LevelFrontend::getContentGfxCache() const {
    return contentCache.get();
}

LevelController* LevelFrontend::getController() const {
    return controller;
}
