#include <logic/LevelController.h>

#include <algorithm>

#include <world/Level.h>
#include <physics/Hitbox.h>
#include <logic/scripting/scripting.h>
#include <world/World.h>
#include <debug/Logger.h>
#include <world/files/WorldFiles.h>
#include <settings.h>
#include <objects/Entities.h>
#include <math/voxmaths.h>
#include <engine/Engine.h>
#include <objects/Players.h>
#include <objects/Player.h>
#include <voxels/Chunks.h>
#include <lighting/Lighting.h>
#include <world/LevelEvents.h>

LevelController::LevelController(
    Engine* engine,
    std::unique_ptr<Level> levelPtr,
    Player* clientPlayer
) : settings(engine->getSettings()),
    level(std::move(levelPtr)),
    chunks(std::make_unique<ChunksController>(*level)),
    playerSparkClock(20, 3)
{
    level->events->listen(LevelEventType::CHUNK_PRESENT, [](auto, Chunk* chunk) {
        scripting::on_chunk_present(*chunk, chunk->flags.loaded);
    });
    level->events->listen(LevelEventType::CHUNK_UNLOAD, [](auto, Chunk* chunk) {
        scripting::on_chunk_remove(*chunk);
    });

    if (clientPlayer) {
        chunks->lighting = std::make_unique<Lighting>(
            level->content, *clientPlayer->chunks
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
            if (!player->isLoadingChunks()) {
                confirmed++;
                continue;
            }
            glm::vec3 position = player->getPosition();
            player->chunks->configure(
                std::floor(position.x), std::floor(position.z), 1
            );
            chunks->update(16, 1, 0, *player);
            if (player->chunks->getVoxel(std::floor(position.x), 0, std::floor(position.z))) {
                confirmed++;
            }
        }
    } while (confirmed < level->players->size());
}

void LevelController::update(float delta, bool pause) {
    for (const auto& [_, player] : *level->players) {
        if (player->isSuspended()) continue;
        player->rotationInterpolation.updateTimer(delta);
        player->updateEntity();
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
            if (player->isSuspended()) continue;
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
    if (world->isNameless()) {
        LOG_WARN("Nameless world will not be saved");
        return;
    }
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
