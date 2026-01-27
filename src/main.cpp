#include <memory>

#include "engine.h"
#include "util/platform.h"
#include "definitions.h"
#include "logger/Logger.h"

// Точка входа в программу
int main() {
    platform::configure_encoding();

    setup_definitions();

    std::unique_ptr<Engine> engine = nullptr;

    try {
        engine = std::make_unique<Engine>(EngineSettings{1280, 720, 0, 1, "ChromaForge", 10, 12, 2});
        engine->mainloop(); // Запуск основного цикла
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred\n{}", err.what());
    }

    Logger::getInstance().flush();

    return 0;
}
