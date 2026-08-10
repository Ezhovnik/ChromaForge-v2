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
#include <voxels/Pathfinding.h>
#include <engine/EnginePaths.h>

static debug::Logger logger("level-controller");

LevelController::LevelController(
    Engine& engine,
    std::unique_ptr<Level> levelPtr,
    Player* clientPlayer
) : engine(engine),
    settings(engine.getSettings()),
    level(std::move(levelPtr)),
    chunks(std::make_unique<ChunksController>(*level)),
    playerSparkClock(20, 3),
    clientPlayer(clientPlayer)
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
            chunks->update(16, 1, 0, *player, player.get() == clientPlayer);
            if (player->chunks->getVoxel(std::floor(position.x), 0, std::floor(position.z))) {
                confirmed++;
            }
        }
    } while (confirmed < level->players->size());
}

void LevelController::update(float delta, bool pause) {
    level->pathfinding->performAllAsync(
        settings.pathfinding.stepsPerAsyncAgent.get()
    );
    for (const auto& [_, player] : *level->players) {
        if (player->isSuspended()) continue;
        player->rotationInterpolation.updateTimer(delta);
        player->updateEntity();
        glm::vec3 position = player->getPosition();
        player->chunks->configure(
            glm::floor(position.x),
            glm::floor(position.z),
            settings.chunks.loadDistance.get() + settings.chunks.padding.get()
        );
        chunks->update(
            settings.chunks.loadSpeed.get(),
            settings.chunks.loadDistance.get(),
            settings.chunks.padding.get(),
            *player,
            player.get() == clientPlayer
        );
    }
    if (!pause) {
        blocks->update(delta, settings.chunks.padding.get());
        level->entities->update(delta);

        for (const auto& [_, player] : *level->players) {
            if (player->isSuspended()) continue;
            if (int parts = playerSparkClock.update(delta)) {
                for (int i = 0; i < parts; ++i) {
                    if (player->getId() % playerSparkClock.getParts() !=
                        playerSparkClock.convertPart(i)) {
                        continue;
                    }
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

void LevelController::processBeforeQuit() {
    preQuitCallbacks.notify();
    for (auto player : level->players->getAll()) {
        if (player->chunks) {
            player->chunks->saveAndClear();
        }
    }
    scripting::process_before_quit();
}

void LevelController::saveWorld() {
    auto world = level->getWorld();
    if (world->isNameless()) {
        logger.warning() << "Nameless world will not be saved";
        return;
    }
    logger.info() << "Writing world '" << world->getName() << "'";
    world->wfile->createDirectories();
    scripting::on_world_save();
    level->onSave();
    level->getWorld()->write(level.get());
    logger.info() << "The world has been successfully saved";
}

void LevelController::onWorldQuit() {
    scripting::on_world_quit();
    engine.getPaths().setCurrentWorldFolder("");
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
