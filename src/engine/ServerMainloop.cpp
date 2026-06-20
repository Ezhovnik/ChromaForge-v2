#include <engine/ServerMainloop.h>

#include <chrono>

#include <engine/Engine.h>
#include <debug/Logger.h>
#include <interfaces/Process.h>
#include <logic/scripting/scripting.h>
#include <logic/LevelController.h>
#include <world/Level.h>
#include <world/World.h>
#include <util/platform.h>

inline constexpr int SPS = 20;

ServerMainloop::ServerMainloop(Engine& engine) : engine(engine) {}

ServerMainloop::~ServerMainloop() = default;

void ServerMainloop::run() {
    const auto& coreParams = engine.getCoreParameters();
    auto& time = engine.getTime();

    if (coreParams.scriptFile.empty()) {
        LOG_INFO("Nothing to do(✿◠‿◠)");
        return;
    }

    engine.setLevelConsumer([this](auto level, auto) {
        setLevel(std::move(level));
    });

    LOG_INFO("Starting task {}", coreParams.scriptFile.u8string());
    auto process = scripting::start_coroutine(coreParams.scriptFile);

    double targetDelta = 1.0 / static_cast<double>(SPS);
    double delta = targetDelta;
    auto begin = std::chrono::system_clock::now();
    auto startupTime = begin;

    while (process->isActive()) {
        if (engine.isQuitSignal()) {
            process->terminate();
            LOG_INFO("Script has been terminated due to quit signal");
            break;
        }
        if (coreParams.testMode) {
            time.step(delta);
        } else {
            auto now = std::chrono::system_clock::now();
            time.update(std::chrono::duration_cast<std::chrono::microseconds>(now - startupTime).count() / 1e6);
            delta = time.getDeltaTime();
        }
        process->update();
        if (controller) {
            controller->getLevel()->getWorld()->updateTimers(delta);
            controller->update(glm::min(delta, 0.2), false);
        }
        engine.postUpdate();

        if (!coreParams.testMode) {
            auto end = std::chrono::system_clock::now();
            int64_t millis = targetDelta * 1000 - std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000;
            if (millis > 0) platform::sleep(millis);
            begin = std::chrono::system_clock::now();
        }
    }
    LOG_INFO("Script finished");
}

void ServerMainloop::setLevel(std::unique_ptr<Level> level) {
    if (level == nullptr) {
        controller->onWorldQuit();
        engine.getPaths().setCurrentWorldFolder(std::filesystem::path());
        controller = nullptr;
    } else {
        controller = std::make_unique<LevelController>(
            &engine, std::move(level), nullptr
        );
    }
}
