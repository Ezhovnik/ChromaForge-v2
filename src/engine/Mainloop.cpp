#include <engine/Mainloop.h>

#include <engine/Engine.h>
#include <window/Window.h>
#include <frontend/screens/MenuScreen.h>
#include <debug/Logger.h>
#include <frontend/screens/LevelScreen.h>
#include <world/Level.h>
#include <devtools/Project.h>
#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/Container.h>

static debug::Logger logger("mainloop");

Mainloop::Mainloop(Engine& engine) : engine(engine) {}

void Mainloop::run() {
    auto& time = engine.getTime();
    auto& window = engine.getWindow();
    auto& settings = engine.getSettings();

    engine.setLevelConsumer([this](auto level, int64_t localPlayer) {
        if (level == nullptr) {
            engine.setScreen(nullptr);
            engine.setScreen(std::make_shared<MenuScreen>(engine));
        } else {
            engine.setScreen(std::make_shared<LevelScreen>(
                engine, std::move(level), localPlayer
            ));
        }
    });

    logger.info() << "Loading the menu screen";
    engine.setScreen(std::make_shared<MenuScreen>(engine));
    logger.info() << "The menu screen has loaded successfully";

    logger.info() << "Main loop started";
    while (!window.isShouldClose()) {
        time.update(window.time());
        engine.applicationSpark();
        engine.updateFrontend();
        if (!window.isIconified()) {
            engine.renderFrame();
        }
        engine.postUpdate();
        engine.nextFrame(
            settings.display.adaptiveFpsInMenu.get() &&
            dynamic_cast<const MenuScreen*>(engine.getScreen().get()) != nullptr
        );
    }
    logger.info() << "Main loop stopped";
}
