#include <memory>
#include <filesystem>
#include <cstdlib>
#include <string>

#include "engine.h"
#include "settings.h"
#include "files/files.h"
#include "util/platform.h"
#include "coders/toml.h"
#include "input_bindings.h"
#include "core_content_defs.h"
#include "debug/Logger.h"
#include "util/command_line.h"
#include "files/settings_io.h"
#include "files/engine_paths.h"
#include "constants.h"
#include "window/Events.h"

inline const std::string SETTINGS_FILE = "settings.toml";
inline const std::string CONTROLS_FILE = "controls.json";

// Точка входа в программу
int main(int argc, char** argv) {
    EnginePaths paths;
	if (!parse_cmdline(argc, argv, paths)) return EXIT_SUCCESS;
    std::filesystem::path userfiles = paths.getUserfiles();

    platform::configure_encoding();

    // Инициализация логгера
	if (ENGINE_DEBUG_BUILD) {
		Logger::getInstance().initialize(paths.getLogsFile().string());
	} else {
		Logger::getInstance().initialize(
			paths.getLogsFile().string(),
			LogLevel::WARN,
			LogLevel::INFO
		);
	}

    std::unique_ptr<Engine> engine = nullptr;

    try {
        // Чтение настроек движка
        EngineSettings settings;
        SettingsHandler handler(settings);
        std::filesystem::path settings_file = userfiles/std::filesystem::path(SETTINGS_FILE);
        if (std::filesystem::is_regular_file(settings_file)) {
			LOG_INFO("Reading engine settings from '{}'", settings_file.string());
			std::string text = files::read_string(settings_file);
			toml::parse(handler, settings_file.string(), text);
            LOG_INFO("Engine settings read succesfully");
		}
        CoreContent::setup_bindings();

        engine = std::make_unique<Engine>(settings, handler, &paths);

        // Настройка назначения клавиш
        std::filesystem::path controls_file = userfiles/std::filesystem::path(CONTROLS_FILE);
		if (std::filesystem::is_regular_file(controls_file)) {
			LOG_INFO("Reading bindings from '{}'", controls_file.string());
			std::string text = files::read_string(controls_file);
			Events::loadBindings(controls_file.string(), text);
            LOG_INFO("Bindings read succesfully");
		}

        engine->mainloop(); // Запуск основного цикла

        // Запись настроек движка в файл
        LOG_INFO("Write engine settings to '{}'", settings_file.string());
		files::write_string(settings_file, toml::stringify(handler));
        LOG_INFO("Engine settings are written");

        // Запись назначения клавиш в файл
        LOG_INFO("Write bindings to '{}'", controls_file.string());
		files::write_string(controls_file, Events::writeBindings());
        LOG_INFO("Bindings are written");
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred: {}", err.what());
    } catch (const std::exception& err) {
        LOG_ERROR("Uncaught exception: {}", err.what());
        throw;
    }

    Logger::getInstance().flush();

    return EXIT_SUCCESS;
}
