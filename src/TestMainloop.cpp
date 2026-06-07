#include <TestMainloop.h>

#include <engine.h>
#include <debug/Logger.h>
#include <interfaces/Process.h>
#include <logic/scripting/scripting.h>

inline constexpr int SPS = 20;

TestMainloop::TestMainloop(Engine& engine) : engine(engine) {}

void TestMainloop::run() {
    const auto& coreParams = engine.getCoreParameters();
    auto& time = engine.getTime();

    if (coreParams.testFile.empty()) {
        LOG_INFO("Nothing to do(✿◠‿◠)");
        return;
    }

    LOG_INFO("Starting task {}", coreParams.testFile.u8string());
    auto process = scripting::start_coroutine(coreParams.testFile);
    while (process->isActive()) {
        time.step(1.0f / static_cast<float>(SPS));
        process->update();
    }
    LOG_INFO("Test finished");
}
