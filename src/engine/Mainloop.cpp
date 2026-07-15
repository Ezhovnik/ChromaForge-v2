#include <engine/Mainloop.h>

#include <engine/Engine.h>
#include <window/Window.h>
#include <frontend/screens/MenuScreen.h>
#include <debug/Logger.h>
#include <frontend/screens/LevelScreen.h>
#include <world/Level.h>
#include <devtools/Project.h>
#include <interfaces/Process.h>
#include <logic/scripting/scripting.h>

Mainloop::Mainloop(Engine& engine) : engine(engine) {}

void Mainloop::run() {
    auto& time = engine.getTime();
    auto& window = engine.getWindow();

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

    io::path projectScript = "project:project_script.lua";
    std::unique_ptr<Process> process;
    if (io::exists(projectScript)) {
        LOG_INFO("Starting project scritp");
        process = scripting::start_coroutine(projectScript);
    } else {
        LOG_WARN("Project script does not exists");
    }

    LOG_INFO("Loading the menu screen");
    engine.setScreen(std::make_shared<MenuScreen>(engine));
    LOG_INFO("The menu screen has loaded successfully");

    LOG_INFO("Main loop started");
    while (!window.isShouldClose()) {
        if (process) process->update();

        time.update(window.time());
        engine.updateFrontend();
        if (!window.isIconified()) {
            engine.renderFrame();
        }
        engine.postUpdate();
        engine.nextFrame();
    }
    LOG_INFO("Main loop stopped");
}
