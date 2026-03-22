#include "LevelController.h"

#include <algorithm>

#include "../world/Level.h"
#include "../physics/Hitbox.h"
#include "scripting/scripting.h"
#include "../world/World.h"
#include "../interfaces/Object.h"
#include "../debug/Logger.h"
#include "../files/WorldFiles.h"

LevelController::LevelController(EngineSettings& settings, Level* level) : 
    settings(settings), 
    level(level),
    blocks(std::make_unique<BlocksController>(level, settings.chunks.padding.get())),
    chunks(std::make_unique<ChunksController>(level, settings.chunks.padding.get())),
    player(std::make_unique<PlayerController>(level, settings, blocks.get()))
{

    scripting::on_world_load(this);
}

void LevelController::update(float delta, bool input, bool pause) {
    player->update(delta, input, pause);
    glm::vec3 position = player->getPlayer()->hitbox->position;
    level->loadMatrix(
        position.x, 
        position.z, 
        settings.chunks.loadDistance.get() + settings.chunks.padding.get() * 2
    );
    chunks->update(settings.chunks.loadSpeed.get());

    level->objects.erase(
        std::remove_if(
            level->objects.begin(), level->objects.end(),
            [](auto obj) { return obj == nullptr; }),
        level->objects.end()
    );

    if (!pause) {
        for(auto obj : level->objects) {
            if(obj && obj->shouldUpdate) obj->update(delta);
        }

        blocks->update(delta);
    }
}

void LevelController::saveWorld() {
    LOG_INFO("Saving world");
    level->getWorld()->wfile->createDirectories();
    scripting::on_world_save();
    level->getWorld()->write(level.get());
    LOG_INFO("The world has been successfully saved");
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
}

Level* LevelController::getLevel() {
    return level.get();
}

Player* LevelController::getPlayer() {
    return player->getPlayer();
}

PlayerController* LevelController::getPlayerController() {
    return player.get();
}

BlocksController* LevelController::getBlocksController() {
    return blocks.get();
}
