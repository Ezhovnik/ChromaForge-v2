#include <engine/WindowControl.h>

#include <engine/Engine.h>
#include <devtools/Project.h>
#include <coders/imageio.h>
#include <window/Window.h>
#include <debug/Logger.h>
#include <window/input.h>
#include <graphics/core/ImageData.h>
#include <constants.h>
#include <engine/EnginePaths.h>
#include <util/platform.h>

namespace {
    static std::unique_ptr<ImageData> load_icon() {
        try {
            auto file = "res:textures/misc/icon.png";
            if (io::exists(file)) {
                return imageio::read(file);
            }
        } catch (const std::exception& err) {
            LOG_ERROR("Could not load window icon: {}", err.what());
        }
        return nullptr;
    }
}

WindowControl::WindowControl(Engine& engine) : engine(engine) {}

WindowControl::Result WindowControl::initialize() {
    const auto& project = engine.getProject();
    auto& settings = engine.getSettings();

    std::string title = project.title;
    if (!title.empty()) title += " - ";
    title += "ChromaForge v" + ENGINE_VERSION_STRING;
    if (ENGINE_DEBUG_BUILD) title += " [development build]";
    if (engine.getDebuggingServer()) title = "[debugging] " + title;

    auto [window, input] = Window::initialize(&settings.display, title);
    if (!window || !input) {
        LOG_CRITICAL("Could not initialize window");
        throw initialize_error("Could not initialize window");
    }

    window->setFramerate(settings.display.framerate.get());
    if (auto icon = load_icon()) {
        icon->flipY();
        window->setIcon(icon.get());
    }

    return Result {std::move(window), std::move(input)};
}

void WindowControl::saveScreenshot() {
    auto& window = engine.getWindow();
    const auto& paths = engine.getPaths();

    auto image = window.takeScreenshot();
    image->flipY();
    io::path filename = paths.getNewScreenshotFile("png");
    imageio::write(filename.string(), image.get());
    LOG_INFO("Save screenshot as '{}'", filename.string());
}

void WindowControl::nextFrame(bool waitForRefresh) {
    const auto& settings = engine.getSettings();
    auto& window = engine.getWindow();
    auto& input = engine.getInput();

    window.setFramerate(
        window.isIconified() && settings.display.limitFpsIconified.get()
            ? 20
            : settings.display.framerate.get()
    );
    window.swapBuffers();
    input.pollEvents(waitForRefresh && !window.checkShouldRefresh());
}

void WindowControl::toggleFullscreen() {
    auto& settings = engine.getSettings();
    auto& windowMode = settings.display.windowMode;

    if (windowMode.get() != static_cast<int>(WindowMode::Fullscreen)) {
        windowMode.set(static_cast<int>(WindowMode::Fullscreen));
    } else {
        windowMode.set(static_cast<int>(WindowMode::Windowed));
    }
}
