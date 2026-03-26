#include "LevelScreen.h"

#include "../hud.h"
#include "../LevelFrontend.h"
#include "../../audio/audio.h"
#include "../../graphics/core/DrawContext.h"
#include "../../graphics/core/Viewport.h"
#include "../../graphics/ui/GUI.h"
#include "../../graphics/ui/elements/layout/Menu.h"
#include "../../graphics/render/WorldRenderer.h"
#include "../../logic/LevelController.h"
#include "../../logic/scripting/scripting_hud.h"
#include "../../physics/Hitbox.h"
#include "../../voxels/Chunks.h"
#include "../../world/Level.h"
#include "../../world/World.h"
#include "../../window/Camera.h"
#include "../../window/Events.h"
#include "../../engine.h"
#include "../../coders/imageio.h"
#include "../../graphics/core/PostProcessing.h"
#include "../../graphics/core/ImageData.h"
#include "../../debug/Logger.h"

LevelScreen::LevelScreen(Engine* engine, Level* level) : Screen(engine) {
    postProcessing = std::make_unique<PostProcessing>();

    auto& settings = engine->getSettings();
    auto assets = engine->getAssets();
    auto menu = engine->getGUI()->getMenu();
    menu->reset();

    controller = std::make_unique<LevelController>(settings, level);
    frontend = std::make_unique<LevelFrontend>(controller.get(), assets);

    worldRenderer = std::make_unique<WorldRenderer>(engine, frontend.get(), controller->getPlayer());
    hud = std::make_unique<Hud>(engine, frontend.get(), controller->getPlayer());
    
    keepAlive(settings.graphics.backlight.observe([=](bool flag) {
        controller->getLevel()->chunks->saveAndClear();
    }));
    keepAlive(settings.camera.fov.observe([=](double value) {
        controller->getPlayer()->camera->setFov(glm::radians(value));
    }));

    animator = std::make_unique<TextureAnimator>();
    animator->addAnimations(assets->getAnimations());

    auto content = level->content;
    for (auto& entry : content->getPacks()) {
        auto pack = entry.second.get();
        const ContentPack& info = pack->getInfo();
        std::filesystem::path scriptFile = info.folder/std::filesystem::path("scripts/hud.lua");
        if (std::filesystem::is_regular_file(scriptFile)) {
            scripting::load_hud_script(pack->getEnvironment(), info.id, scriptFile);
        }
    }
    scripting::on_frontend_init(hud.get());
}

LevelScreen::~LevelScreen() {
    saveWorldPreview();
    scripting::on_frontend_close();
    controller->onWorldQuit();
    engine->getPaths()->setWorldFolder(std::filesystem::path());
}

void LevelScreen::saveWorldPreview() {
    try {
        LOG_INFO("Saving world preview");
        auto paths = engine->getPaths();
        auto player = controller->getPlayer();
        auto& settings = engine->getSettings();
        int previewSize = settings.ui.worldPreviewSize.get();

        Camera camera = *player->camera;
        camera.setFov(glm::radians(70.0f));
        Viewport viewport(previewSize * 1.5, previewSize);
        DrawContext ctx(nullptr, viewport, batch.get());
        worldRenderer->draw(ctx, &camera, false, postProcessing.get());
        auto image = postProcessing->toImage();
        image->flipY();
        imageio::write(paths->resolve("world:preview.png").string(), image.get());
        LOG_INFO("World preview successfully saved");
    } catch (const std::exception& err) {
        LOG_ERROR("Failed to save world preview: {}", err.what());
    }
}

void LevelScreen::updateHotkeys() {
    auto& settings = engine->getSettings();

    if (Events::justPressed(keycode::F1)) hudVisible = !hudVisible;
    if (Events::justPressed(keycode::F3)) controller->getPlayer()->debug = !controller->getPlayer()->debug;
    if (Events::justPressed(keycode::F5)) controller->getLevel()->chunks->saveAndClear();
}

void LevelScreen::update(float deltaTime) {
    gui::GUI* gui = engine->getGUI();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!gui->isFocusCaught()) updateHotkeys();

    auto player = controller->getPlayer();
    auto camera = player->camera;

    bool paused = hud->isPause();
    audio::get_channel("regular")->setPaused(paused);
    audio::get_channel("ambient")->setPaused(paused);
    audio::set_listener(
        camera->position-camera->dir, 
        player->hitbox->velocity,
        camera->dir, 
        glm::vec3(0, 1, 0)
    );

    if (!hud->isPause()) {
        controller->getLevel()->getWorld()->updateTimers(deltaTime);
        animator->update(deltaTime);
    }
    controller->update(deltaTime, !inputLocked, hud->isPause());
    hud->update(hudVisible);
}

void LevelScreen::draw(float deltaTime) {
    auto camera = controller->getPlayer()->currentCamera;

    Viewport viewport(Window::width, Window::height);
    DrawContext ctx(nullptr, viewport, batch.get());

    worldRenderer->draw(ctx, camera.get(), hudVisible, postProcessing.get());

    if (hudVisible) hud->draw(ctx);
}

void LevelScreen::onEngineShutdown() {
    controller->saveWorld();
}

LevelController* LevelScreen::getLevelController() const {
    return controller.get();
}
