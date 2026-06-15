#include <ServerMainloop.h>

#include <chrono>

#include <engine.h>
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

    engine.setLevelConsumer([this](auto level) {
        setLevel(std::move(level));
    });

    LOG_INFO("Starting task {}", coreParams.scriptFile.u8string());
    auto process = scripting::start_coroutine(coreParams.scriptFile);

    double targetDelta = 1.0f / static_cast<float>(SPS);
    double delta = targetDelta;
    auto begin = std::chrono::steady_clock::now();
    while (process->isActive()) {
        if (engine.isQuitSignal()) {
            process->terminate();
            LOG_INFO("Script has been terminated due to quit signal");
            break;
        }
        time.step(delta);
        process->update();
        if (controller) {
            float deltaTime = time.getDeltaTime();
            controller->getLevel()->getWorld()->updateTimers(deltaTime);
            controller->update(glm::min(deltaTime, 0.2f), false);
        }

        if (!coreParams.testMode) {
            auto end = std::chrono::steady_clock::now();
            platform::sleep(targetDelta * 1000 - std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000);
            end = std::chrono::steady_clock::now();
            delta = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1e6;
            begin = end;
        }
    }
    LOG_INFO("Test finished");
}

void ServerMainloop::setLevel(std::unique_ptr<Level> level) {
    if (level == nullptr) {
        controller->onWorldQuit();
        engine.getPaths()->setCurrentWorldFolder(std::filesystem::path());
        controller = nullptr;
    } else {
        controller = std::make_unique<LevelController>(
            &engine, std::move(level), nullptr
        );
    }
}
