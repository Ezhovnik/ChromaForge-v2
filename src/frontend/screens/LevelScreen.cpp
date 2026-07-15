#include <frontend/screens/LevelScreen.h>

#include <frontend/hud.h>
#include <frontend/LevelFrontend.h>
#include <audio/audio.h>
#include <graphics/core/DrawContext.h>
#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/Menu.h>
#include <graphics/render/WorldRenderer.h>
#include <logic/LevelController.h>
#include <logic/scripting/scripting_hud.h>
#include <physics/Hitbox.h>
#include <voxels/Chunks.h>
#include <world/Level.h>
#include <world/World.h>
#include <window/Camera.h>
#include <window/Window.h>
#include <engine/Engine.h>
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
#include <frontend/ContentGfxCache.h>
#include <voxels/GlobalChunks.h>
#include <assets/Assets.h>
#include <graphics/core/TextureAnimation.h>

inline const io::path CLIENT_FILE = "world:client/environment.json";

LevelScreen::LevelScreen(
    Engine& engine,
    std::unique_ptr<Level> levelPtr,
    int64_t localPlayer
) : Screen(engine),
    world(*levelPtr->getWorld()),
    postProcessing(std::make_unique<PostProcessing>(
        levelPtr->content.getIndices(ResourceType::PostEffectSlot).size()
    )),
    gui(engine.getGUI()),
    input(engine.getInput())
{
    Level* level = levelPtr.get();

    auto& settings = engine.getSettings();
    auto& assets = *engine.getAssets();
    auto menu = engine.getGUI().getMenu();
    menu->reset();

    auto player = level->players->getPlayer(localPlayer);
    assert(player != nullptr);

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
        engine, player, controller.get(), settings
    );

    renderer = std::make_unique<WorldRenderer>(
        engine, *frontend, *player
    );
    hud = std::make_unique<Hud>(engine, *frontend, *player);

    decorator = std::make_unique<Decorator>(
        engine, *controller, *renderer, assets, *player
    );

    keepAlive(settings.graphics.backlight.observe([=](bool) {
        player->chunks->saveAndClear();
        renderer->clear();
    }));
    keepAlive(settings.graphics.denseRender.observe([=](bool) {
        player->chunks->saveAndClear();
        renderer->clear();
        frontend->getContentGfxCache().refresh();
    }));
    keepAlive(settings.camera.fov.observe([=](double value) {
        player->fpCamera->setFov(glm::radians(value));
    }));
    keepAlive(input.addCallback(BIND_CHUNKS_RELOAD, [=](){
        player->chunks->saveAndClear();
        renderer->clear();
        return false;
    }));

    animator = std::make_unique<TextureAnimator>();
    animator->addAnimations(assets.getAnimations());

    loadDecorations();
    initializeContent();
}

LevelScreen::~LevelScreen() {
    if (!controller->getLevel()->getWorld()->isNameless()) {
        saveDecorations();
        saveWorldPreview();
    }
    scripting::on_frontend_close();
    input.getBindings().enableAll();
    controller->onWorldQuit();
    engine.getPaths().setCurrentWorldFolder("");
}

void LevelScreen::initializeContent() {
    auto& content = controller->getLevel()->content;
    for (auto& entry : content.getPacks()) {
        LOG_INFO("Initializing pack '{}'", entry.first);
        initializePack(entry.second.get());
    }
    scripting::on_frontend_init(
        hud.get(), renderer.get(), postProcessing.get()
    );
}

void LevelScreen::initializePack(ContentPackRuntime* pack) {
    const ContentPack& info = pack->getInfo();
    io::path scriptFile = info.folder / "scripts/hud.lua";
    if (io::is_regular_file(scriptFile)) {
        scripting::load_hud_script(
            pack->getEnvironment(),
            info.id,
            scriptFile,
            pack->getId() + ":scripts/hud.lua"
        );
    }
}

void LevelScreen::loadDecorations() {
    if (!io::exists(CLIENT_FILE)) return;

    auto data = io::read_object(CLIENT_FILE);
    if (data.has("weather")) {
        renderer->getWeather().deserialize(data["weather"]);
    }
}

void LevelScreen::saveDecorations() {
    io::create_directory("world:client");

    auto data = dv::object();
    data["weather"] = renderer->getWeather().serialize();
    io::write_json(CLIENT_FILE, data, true);
}

void LevelScreen::saveWorldPreview() {
    try {
        LOG_INFO("Saving world preview");
        auto player = playerController->getPlayer();
        auto& settings = engine.getSettings();
        int previewSize = settings.ui.worldPreviewSize.get();

        Camera camera = *player->fpCamera;
        camera.setFov(glm::radians(70.0f));

        DrawContext parent_ctx(nullptr, engine.getWindow(), batch.get());

        DrawContext ctx(&parent_ctx, engine.getWindow(), batch.get());
        ctx.setViewport(
            {static_cast<uint>(previewSize * 1.5), static_cast<uint>(previewSize)}
        );

        renderer->renderFrame(ctx, camera, false, true, 0.0f, *postProcessing);
        auto image = postProcessing->toImage();
        image->flipY();
        imageio::write("world:preview.png", image.get());
        LOG_INFO("World preview successfully saved");
    } catch (const std::exception& err) {
        LOG_ERROR("Failed to save world preview: {}", err.what());
    }
}

void LevelScreen::saveWorldPanorama() {
    auto& settings = engine.getSettings();
    int panoramaSize = settings.ui.worldPanoramaSize.get();
    auto player = playerController->getPlayer();
    glm::vec3 eyePos = player->fpCamera->position;

    const struct {
        glm::vec3 dir;
        glm::vec3 up;
        int index;
    } faces[6] = {
        {glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),  0},
        {glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0),  1},
        {glm::vec3(0, 1, 0), glm::vec3(0, 0, 1),  2},
        {glm::vec3(0, -1, 0), glm::vec3(0, 0, -1),  3},
        {glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),  4},
        {glm::vec3(0, 0, 1), glm::vec3(0, 1, 0),  5}
    };

    auto& paths = engine.getPaths();
    auto panorama_folder = paths.getNewPanoramaFolder();

    for (const auto& face : faces) {
        Camera cam(eyePos, glm::radians(90.0f));
        cam.front = face.dir;
        cam.up = face.up;
        cam.perspective = true;
        cam.setAspectRatio(1.0f);

        DrawContext parent_ctx(nullptr, engine.getWindow(), batch.get());

        DrawContext ctx(&parent_ctx, engine.getWindow(), batch.get());
        ctx.setViewport(
            {static_cast<uint>(panoramaSize), static_cast<uint>(panoramaSize)}
        );

        renderer->renderFrame(ctx, cam, false, true, 0.0f, *postProcessing);
        auto image = postProcessing->toImage();
        image->flipY();

        imageio::write(panorama_folder / (std::to_string(face.index) + ".png"), image.get());
    }

    LOG_INFO("Panorama saved to {}", panorama_folder.string());
}

void LevelScreen::updateHotkeys() {
    auto& settings = engine.getSettings();

    if (input.justPressed(Keycode::F1)) hudVisible = !hudVisible;
    if (!input.isPressed(Keycode::LEFT_CONTROL)) {
        if (input.justPressed(Keycode::F3)) {
            debug = !debug;
            hud->setDebug(debug);
            renderer->setDebug(debug);
        }
    } else if (input.isPressed(Keycode::F3)) {
        if (input.justPressed(Keycode::L)) {
            renderer->toggleLightsDebug();
        }
    }
    if (input.isPressed(Keycode::F12)) saveWorldPanorama();
}

void LevelScreen::updateAudio() {
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
}

void LevelScreen::update(float deltaTime) {
    auto& gui = engine.getGUI();

    if (!gui.isFocusCaught()) {
        updateHotkeys();
    }

    updateAudio();

    auto menu = gui.getMenu();
    bool inputLocked = menu->hasOpenPage() || hud->isInventoryOpen() || gui.isFocusCaught();
    bool paused = hud->isPause();
    if (!paused) {
        world.updateTimers(deltaTime);
        animator->update(deltaTime);
        playerController->update(deltaTime, inputLocked ? nullptr : &engine.getInput());
    }
    controller->update(glm::min(deltaTime, 0.2f), paused);
    playerController->postUpdate(
        deltaTime,
        engine.getWindow().getSize().y,
        inputLocked ? nullptr : &engine.getInput(),
        paused
    );

    hud->update(hudVisible);

    const auto& weather = renderer->getWeather();
    const auto& player = *playerController->getPlayer();
    const auto& camera = *player.currentCamera;
    decorator->update(paused ? 0.0f : deltaTime, camera, weather);
}

void LevelScreen::draw(float deltaTime) {
    auto camera = playerController->getPlayer()->currentCamera;

    DrawContext ctx(nullptr, engine.getWindow(), batch.get());

    if (!hud->isPause()) {
        scripting::on_entities_render(engine.getTime().getDeltaTime());
    }

    renderer->renderFrame(
        ctx,
        *camera,
        hudVisible,
        hud->isPause(),
        deltaTime,
        *postProcessing
    );

    if (hudVisible) hud->draw(ctx);
}

void LevelScreen::onEngineShutdown() {
    if (hud->isInventoryOpen()) {
        hud->closeInventory();
    }
    controller->saveWorld();
}
