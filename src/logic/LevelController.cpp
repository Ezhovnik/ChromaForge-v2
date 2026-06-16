#include <logic/LevelController.h>

#include <algorithm>

#include <world/Level.h>
#include <physics/Hitbox.h>
#include <logic/scripting/scripting.h>
#include <world/World.h>
#include <debug/Logger.h>
#include <files/WorldFiles.h>
#include <settings.h>
#include <objects/Entities.h>
#include <math/voxmaths.h>
#include <engine.h>
#include <objects/Players.h>
#include <objects/Player.h>
#include <voxels/Chunks.h>
#include <lighting/Lighting.h>

LevelController::LevelController(
    Engine* engine,
    std::unique_ptr<Level> levelPtr,
    Player* clientPlayer
) : settings(engine->getSettings()),
    level(std::move(levelPtr)),
    chunks(std::make_unique<ChunksController>(*level)),
    playerSparkClock(20, 3)
{
    if (clientPlayer) {
        chunks->lighting = std::make_unique<Lighting>(
            level->content, clientPlayer->chunks.get()
        );
    }

    blocks = std::make_unique<BlocksController>(
        *level, 
        chunks ? chunks->lighting.get() : nullptr
    );
    scripting::on_world_load(this);

    // TODO: players added later
    int confirmed;
    do {
        confirmed = 0;
        for (const auto& [_, player] : *level->players) {
            glm::vec3 position = player->getPosition();
            player->chunks->configure(
                std::floor(position.x), std::floor(position.z), 1
            );
            chunks->update(16, 1, 0, *player);
            if (player->chunks->getVoxel(std::floor(position.x), std::floor(position.y), std::floor(position.z))) {
                confirmed++;
            }
        }
    } while (confirmed < level->players->size());
}

void LevelController::update(float delta, bool pause) {
    for (const auto& [_, player] : *level->players) {
        glm::vec3 position = player->getPosition();
        player->chunks->configure(
            position.x,
            position.z,
            settings.chunks.loadDistance.get() + settings.chunks.padding.get()
        );
        chunks->update(
            settings.chunks.loadSpeed.get(),
            settings.chunks.loadDistance.get(),
            settings.chunks.padding.get(),
            *player
        );
    }
    if (!pause) {
        blocks->update(delta, settings.chunks.padding.get());
        level->entities->updatePhysics(delta);
        level->entities->update(delta);

        for (const auto& [_, player] : *level->players) {
            if (playerSparkClock.update(delta)) {
                if (player->getId() % playerSparkClock.getParts() == playerSparkClock.getPart()) {

                    const auto& position = player->getPosition();
                    if (player->chunks->getVoxel(std::floor(position.x), std::floor(position.y), std::floor(position.z))){
                        scripting::on_player_spark(
                            player.get(), playerSparkClock.getSparkRate()
                        );
                    }
                }
            }
        }
    }
    level->entities->clean();
}

void LevelController::saveWorld() {
    auto world = level->getWorld();
    LOG_INFO("Writing world '{}'", world->getName());
    world->wfile->createDirectories();
    scripting::on_world_save();
    level->onSave();
    level->getWorld()->write(level.get());
    LOG_INFO("The world has been successfully saved");
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
}

Level* LevelController::getLevel() {
    return level.get();
}

BlocksController* LevelController::getBlocksController() {
    return blocks.get();
}

ChunksController* LevelController::getChunksController() {
    return chunks.get();
}
