#include "screens.h"

#include <memory>

#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../assets/Assets.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../objects/Player.h"
#include "../voxels/ChunksController.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "world_render.h"
#include "hud.h"
#include "gui/GUI.h"
#include "../engine.h"
#include "../logger/Logger.h"

LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine), level(level) {
    worldRenderer = new WorldRenderer(level, engine->getAssets());
    hud = new HudRenderer(engine->getGUI(), level, engine->getAssets());
}

LevelScreen::~LevelScreen() {
    delete hud;
    delete worldRenderer;

    LOG_INFO("World saving");
    World* world = level->world;
	world->write(level, !engine->getSettings().debug.generatorTestMode);

	delete world;
    delete level;
    LOG_INFO("The world has been successfully saved");
}

void LevelScreen::updateHotkeys() {
    if (Events::justPressed(keycode::O)) occlusion = !occlusion;

    if (Events::justPressed(keycode::F3)) level->player->debug = !level->player->debug;

    if (Events::justPressed(keycode::F5)) {
        for (uint i = 0; i < level->chunks->volume; i++) {
            std::shared_ptr<Chunk> chunk = level->chunks->chunks[i];
            if (chunk != nullptr && chunk->isReady()) {
                chunk->setModified(true);
            }
        }
    }
}

void LevelScreen::update(float delta) {
    gui::GUI* gui = engine->getGUI();
    EngineSettings& settings = engine->getSettings();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!inputLocked) updateHotkeys();

    level->updatePlayer(delta, !inputLocked, hud->isPause(), !inputLocked);
    level->update();
    level->chunksController->update(settings.chunks.loadSpeed);
}

void LevelScreen::draw(float deltaTime) {
    EngineSettings& settings = engine->getSettings();
    Camera* camera = level->player->camera;

    float fogFactor = 18.0f / (float)settings.chunks.loadDistance;
    worldRenderer->draw(camera, occlusion, fogFactor, settings.graphics.fogCurve);
    hud->draw();
    if (level->player->debug) hud->drawDebug(1 / deltaTime, occlusion);
}
