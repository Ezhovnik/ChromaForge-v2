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

LevelController::LevelController(Engine* engine, std::unique_ptr<Level> levelPtr) : 
    settings(engine->getSettings()),
    level(std::move(levelPtr)),
    blocks(std::make_unique<BlocksController>(*level, settings.chunks.padding.get())),
    chunks(std::make_unique<ChunksController>(*level, settings.chunks.padding.get()))
{
    scripting::on_world_load(this);
}

void LevelController::update(float delta, bool pause) {
    if (!pause) {
        blocks->update(delta);
        level->entities->updatePhysics(delta);
        level->entities->update(delta);
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
