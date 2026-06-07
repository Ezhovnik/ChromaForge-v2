#include <Mainloop.h>

#include <engine.h>
#include <window/Window.h>
#include <frontend/screens/MenuScreen.h>
#include <debug/Logger.h>

Mainloop::Mainloop(Engine& engine) : engine(engine) {}

void Mainloop::run() {
    auto& time = engine.getTime();

    LOG_INFO("Loading the menu screen");
    engine.setScreen(std::make_shared<MenuScreen>(&engine));
    LOG_INFO("The menu screen has loaded successfully");

    LOG_INFO("Main loop started");
    while (!Window::isShouldClose()) {
        time.update(Window::time());
        engine.updateFrontend();
        if (!Window::isIconified()) {
            engine.renderFrame();
        }
        engine.postUpdate();
        engine.nextFrame();
    }
    LOG_INFO("Main loop stopped");
}
