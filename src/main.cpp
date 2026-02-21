#include <memory>
#include <filesystem>

#include "engine.h"
#include "settings.h"
#include "files/files.h"
#include "util/platform.h"
#include "coders/toml.h"
#include "coders/json.h"
#include "definitions.h"
#include "logger/Logger.h"
#include "util/command_line.h"
#include "window/Events.h"
#include "window/input.h"
#include "files/settings_io.h"
#include "files/engine_paths.h"

// Точка входа в программу
int main(int argc, char** argv) {
    EnginePaths paths;
	if (!parse_cmdline(argc, argv, paths)) return EXIT_SUCCESS;

    platform::configure_encoding();

    // Инициализация логгера
    Logger::getInstance().initialize(paths.getLogsFile().string());

    std::unique_ptr<Engine> engine = nullptr;

    try {
        // Чтение настроек движка
        EngineSettings settings;
        std::unique_ptr<toml::Wrapper> wrapper (create_wrapper(settings));
        std::filesystem::path settings_file = platform::get_settings_file();
        if (std::filesystem::is_regular_file(settings_file)) {
			LOG_INFO("Reading engine settings from '{}'", settings_file.string());
			std::string text = files::read_string(settings_file);
			toml::Reader reader(wrapper.get(), settings_file.string(), text);
			reader.read();
            LOG_INFO("Engine settings read succesfully");
		}
        engine = std::make_unique<Engine>(settings, &paths);

        // Настройка назначения клавиш
        std::filesystem::path controls_file = platform::get_controls_file();
        setup_bindings();
		if (std::filesystem::is_regular_file(controls_file)) {
			LOG_INFO("Reading bindings from '{}'", controls_file.string());
			std::string text = files::read_string(controls_file);
			load_controls(controls_file.string(), text);
            LOG_INFO("Bindings read succesfully");
		}

        engine->mainloop(); // Запуск основного цикла

        // Запись настроек движка в файл
        LOG_INFO("Write engine settings to '{}'", settings_file.string());
		files::write_string(settings_file, wrapper->write());
        LOG_INFO("Engine settings are written");

        // Запись назначения клавиш в файл
        LOG_INFO("Write bindings to '{}'", controls_file.string());
		files::write_string(controls_file, write_controls());
        LOG_INFO("Bindings are written");
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred. Reason: {}", err.what());
    }

    Logger::getInstance().flush();

    return EXIT_SUCCESS;
}
