#include <frontend/screens/LevelScreen.h>

#include <frontend/hud.h>
#include <frontend/LevelFrontend.h>
#include <audio/audio.h>
#include <graphics/core/DrawContext.h>
#include <graphics/core/Viewport.h>
#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/layout/Menu.h>
#include <graphics/render/WorldRenderer.h>
#include <logic/LevelController.h>
#include <logic/scripting/scripting_hud.h>
#include <physics/Hitbox.h>
#include <voxels/Chunks.h>
#include <world/Level.h>
#include <world/World.h>
#include <window/Camera.h>
#include <window/Events.h>
#include <engine.h>
#include <coders/imageio.h>
#include <graphics/core/PostProcessing.h>
#include <graphics/core/ImageData.h>
#include <debug/Logger.h>
#include <settings.h>
#include <input_bindings.h>
#include <content/Content.h>
#include <graphics/render/Decorator.h>
#include <logic/scripting/scripting.h>
#include <math/voxmaths.h>
#include <objects/Player.h>
#include <logic/PlayerController.h>
#include <objects/Players.h>

LevelScreen::LevelScreen(
    Engine& engine,
    std::unique_ptr<Level> levelPtr
) : Screen(engine),
    postProcessing(std::make_unique<PostProcessing>())
{
    Level* level = levelPtr.get();

    auto& settings = engine.getSettings();
    auto& assets = *engine.getAssets();
    auto menu = engine.getGUI()->getMenu();
    menu->reset();

    auto player = level->players->getPlayer(0);
    controller = std::make_unique<LevelController>(
        &engine, std::move(levelPtr), player
    );
    playerController = std::make_unique<PlayerController>(
        settings,
        *level,
        *player,
        *controller->getBlocksController()
    );

    frontend = std::make_unique<LevelFrontend>(
        player, controller.get(), assets
    );

    worldRenderer = std::make_unique<WorldRenderer>(
        engine, *frontend, *player
    );
    hud = std::make_unique<Hud>(engine, *frontend, *player);

    decorator = std::make_unique<Decorator>(
        engine, *controller, *worldRenderer, assets, *player
    );

    keepAlive(settings.graphics.backlight.observe([=](bool) {
        player->chunks->saveAndClear();
        worldRenderer->clear();
    }));
    keepAlive(settings.camera.fov.observe([=](double value) {
        player->fpCamera->setFov(glm::radians(value));
    }));
    keepAlive(Events::getBinding(BIND_CHUNKS_RELOAD).onactived.add([=](){
        player->chunks->saveAndClear();
        worldRenderer->clear();
    }));

    animator = std::make_unique<TextureAnimator>();
    animator->addAnimations(assets.getAnimations());

    initializeContent();
}

LevelScreen::~LevelScreen() {
    saveWorldPreview();
    scripting::on_frontend_close();
    Events::enableBindings();
    controller->onWorldQuit();
    engine.getPaths().setCurrentWorldFolder(std::filesystem::path());
}

void LevelScreen::initializeContent() {
    auto content = controller->getLevel()->content;
    for (auto& entry : content->getPacks()) {
        initializePack(entry.second.get());
    }
    scripting::on_frontend_init(hud.get(), worldRenderer.get());
}

void LevelScreen::initializePack(ContentPackRuntime* pack) {
    const ContentPack& info = pack->getInfo();
    std::filesystem::path scriptFile = info.folder/std::filesystem::path("scripts/hud.lua");
    if (std::filesystem::is_regular_file(scriptFile)) {
        scripting::load_hud_script(
            pack->getEnvironment(),
            info.id,
            scriptFile,
            pack->getId() + ":scripts/hud.lua"
        );
    }
}

void LevelScreen::saveWorldPreview() {
    try {
        LOG_INFO("Saving world preview");
        const auto& paths = engine.getPaths();
        auto player = playerController->getPlayer();
        auto& settings = engine.getSettings();
        int previewSize = settings.ui.worldPreviewSize.get();

        Camera camera = *player->fpCamera;
        camera.setFov(glm::radians(70.0f));
        Viewport viewport(previewSize * 1.5, previewSize);
        DrawContext parent_ctx(nullptr, {Window::width, Window::height}, batch.get());
        DrawContext ctx(&parent_ctx, viewport, batch.get());
        worldRenderer->draw(ctx, camera, false, true, 0.0f, postProcessing.get());
        auto image = postProcessing->toImage();
        image->flipY();
        imageio::write(paths.resolve("world:preview.png").string(), image.get());
        LOG_INFO("World preview successfully saved");
    } catch (const std::exception& err) {
        LOG_ERROR("Failed to save world preview: {}", err.what());
    }
}

void LevelScreen::updateHotkeys() {
    auto player = playerController->getPlayer();
    auto& settings = engine.getSettings();

    if (Events::justPressed(keycode::F1)) hudVisible = !hudVisible;
    if (Events::justPressed(keycode::F3)) player->debug = !player->debug;
}

void LevelScreen::update(float deltaTime) {
    gui::GUI* gui = engine.getGUI();

    bool inputLocked = hud->isPause() || hud->isInventoryOpen() || gui->isFocusCaught();
    if (!gui->isFocusCaught()) updateHotkeys();

    auto player = playerController->getPlayer();
    auto camera = player->currentCamera;

    bool paused = hud->isPause();
    audio::get_channel("regular")->setPaused(paused);
    audio::get_channel("ambient")->setPaused(paused);
    glm::vec3 velocity {};
    if (auto hitbox = player->getHitbox())  {
        velocity = hitbox->velocity;
    }
    audio::set_listener(
        camera->position, 
        velocity,
        camera->dir, 
        glm::vec3(0, 1, 0)
    );

    auto level = controller->getLevel();
    const auto& settings = engine.getSettings();

    if (!hud->isPause()) {
        level->getWorld()->updateTimers(deltaTime);
        animator->update(deltaTime);
    }

    if (!hud->isPause()) {
        playerController->update(deltaTime, !inputLocked);
    }
    controller->update(glm::min(deltaTime, 0.2f), hud->isPause());
    playerController->postUpdate(deltaTime, !inputLocked, hud->isPause());


    hud->update(hudVisible);
    decorator->update(deltaTime, *camera);
}

void LevelScreen::draw(float deltaTime) {
    auto camera = playerController->getPlayer()->currentCamera;

    Viewport viewport(Window::width, Window::height);
    DrawContext ctx(nullptr, viewport, batch.get());

    if (!hud->isPause()) {
        scripting::on_entities_render(engine.getTime().getDeltaTime());
    }

    worldRenderer->draw(
        ctx,
        *camera,
        hudVisible,
        hud->isPause(),
        deltaTime,
        postProcessing.get()
    );

    if (hudVisible) hud->draw(ctx);
}

void LevelScreen::onEngineShutdown() {
    if (hud->isInventoryOpen()) {
        hud->closeInventory();
    }
    controller->saveWorld();
}

LevelController* LevelScreen::getLevelController() const {
    return controller.get();
}
