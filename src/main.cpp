#include <iostream>
#include <filesystem>
#include <memory>

#include "settings.h"
#include "engine.h"
#include "definitions.h"
#include "util/platform.h"
#include "logger/Logger.h"
#include "logger/OpenGL_Logger.h"

// Точка входа в программу
int main() {
    platform::configure_encoding();

    // Инициализация логгера
    Logger::getInstance().initialize();

    setup_definitions();

    std::unique_ptr<Engine> engine = nullptr;

    try {
        EngineSettings settings;

        std::string settings_file = platform::get_settings_file();
        if (std::filesystem::is_regular_file(settings_file)) {
			LOG_INFO("Reading engine settings from {}", settings_file);
			read_settings(settings, settings_file);
		} else {
            LOG_INFO("Write engine settings to {}", settings_file);
			write_settings(settings, settings_file);
		}

        engine = std::make_unique<Engine>(settings);
        engine->mainloop(); // Запуск основного цикла
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred\n{}", err.what());
    }

    return 0;
}
