#include <TestMainloop.h>

#include <engine.h>
#include <debug/Logger.h>
#include <interfaces/Process.h>
#include <logic/scripting/scripting.h>
#include <logic/LevelController.h>
#include <world/Level.h>
#include <world/World.h>

inline constexpr int SPS = 20;

TestMainloop::TestMainloop(Engine& engine) : engine(engine) {}

TestMainloop::~TestMainloop() = default;

void TestMainloop::run() {
    const auto& coreParams = engine.getCoreParameters();
    auto& time = engine.getTime();

    if (coreParams.testFile.empty()) {
        LOG_INFO("Nothing to do(✿◠‿◠)");
        return;
    }

    engine.setLevelConsumer([this](auto level) {
        setLevel(std::move(level));
    });

    LOG_INFO("Starting task {}", coreParams.testFile.u8string());
    auto process = scripting::start_coroutine(coreParams.testFile);
    while (process->isActive()) {
        time.step(1.0f / static_cast<float>(SPS));
        process->update();
        if (controller) {
            float deltaTime = time.getDeltaTime();
            controller->getLevel()->getWorld()->updateTimers(deltaTime);
            controller->update(glm::min(deltaTime, 0.2f), false);
        }
    }
    LOG_INFO("Test finished");
}

void TestMainloop::setLevel(std::unique_ptr<Level> level) {
    if (level == nullptr) {
        controller->onWorldQuit();
        engine.getPaths()->setCurrentWorldFolder(std::filesystem::path());
        controller = nullptr;
    } else {
        controller = std::make_unique<LevelController>(&engine, std::move(level));
    }
}
