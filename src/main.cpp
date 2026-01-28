#include <memory>
#include <filesystem>

#include "engine.h"
#include "settings.h"
#include "files/files.h"
#include "util/platform.h"
#include "coders/toml.h"
#include "definitions.h"
#include "logger/Logger.h"
#include "files/engine_files.h"

toml::Wrapper create_wrapper(EngineSettings& settings) {
	toml::Wrapper wrapper;

	toml::Section& display = wrapper.add("display");
	display.add("width", &settings.display.width);
	display.add("height", &settings.display.height);
	display.add("samples", &settings.display.samples);
	display.add("swap-interval", &settings.display.swapInterval);

	toml::Section& chunks = wrapper.add("chunks");
	chunks.add("load-distance", &settings.chunks.loadDistance);
	chunks.add("load-speed", &settings.chunks.loadSpeed);
	chunks.add("padding", &settings.chunks.padding);

    toml::Section& camera = wrapper.add("camera");
	camera.add("fov-events", &settings.camera.fovEvents);
	camera.add("shaking", &settings.camera.shaking);

	toml::Section& graphics = wrapper.add("graphics");
	graphics.add("fog-curve", &settings.graphics.fogCurve);

    toml::Section& debug = wrapper.add("debug");
	graphics.add("generator-test-mode", &settings.debug.generatorTestMode);

	return wrapper;
}

// Точка входа в программу
int main() {
    platform::configure_encoding();

    // Инициализация логгера
    Logger::getInstance().initialize(engine_fs::get_logs_file().string());

    setup_definitions();

    std::unique_ptr<Engine> engine = nullptr;

    try {
        EngineSettings settings;
        toml::Wrapper wrapper = create_wrapper(settings);

        std::filesystem::path settings_file = platform::get_settings_file();
        if (std::filesystem::is_regular_file(settings_file)) {
			LOG_INFO("Reading engine settings from '{}'", settings_file.string());
			std::string content = files::read_string(settings_file.string());
			toml::Reader reader(&wrapper, settings_file.string(), content);
			reader.read();
		} else {
            LOG_INFO("Creating settings file '{}'", settings_file.string());
			files::write_string(settings_file.string(), wrapper.write());
		}

        engine = std::make_unique<Engine>(settings);
        engine->mainloop(); // Запуск основного цикла

        LOG_INFO("Write engine settings to '{}'", settings_file.string());
		files::write_string(settings_file.string(), wrapper.write());
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred\n{}", err.what());
    }

    Logger::getInstance().flush();

    return 0;
}
