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
#include "files/engine_files.h"
#include "window/Events.h"
#include "window/input.h"

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
	debug.add("generator-test-mode", &settings.debug.generatorTestMode);

	return wrapper;
}

std::string write_controls() {
	json::JObject* obj = new json::JObject();
	for (auto& [name, binding] : Events::bindings) {
		json::JObject* jentry = new json::JObject();
		switch (binding.type) {
			case inputType::keyboard: jentry->put("type", "keyboard"); break;
			case inputType::mouse: jentry->put("type", "mouse"); break;
			default: throw std::runtime_error("unsupported control type");
		}
		jentry->put("code", binding.code);
		obj->put(name, jentry);
	}
	return json::stringify(obj, true, "  ");
}

void load_controls(std::string filename, std::string source) {
	json::JObject* obj = json::parse(filename, source);
	for (auto& [name, binding] : Events::bindings) {
		json::JObject* jentry = obj->obj(name);
		if (jentry == nullptr) continue;
		inputType type;
		std::string typestr;
		jentry->str("type", typestr);

		if (typestr == "keyboard") {
			type = inputType::keyboard;
		} else if (typestr == "mouse") {
			type = inputType::mouse;
		} else {
            LOG_WARN("Unknown input type {}", typestr);
			continue;
		}
		binding.type = type;
		jentry->num("code", binding.code);
	}
}

// Точка входа в программу
int main() {
    platform::configure_encoding();

    // Инициализация логгера
    Logger::getInstance().initialize(engine_fs::get_logs_file().string());

    setup_definitions();

    std::unique_ptr<Engine> engine = nullptr;

    try {
        // Чтение настроек движка
        EngineSettings settings;
        toml::Wrapper wrapper = create_wrapper(settings);
        std::filesystem::path settings_file = platform::get_settings_file();
        if (std::filesystem::is_regular_file(settings_file)) {
			LOG_INFO("Reading engine settings from '{}'", settings_file.string());
			std::string content = files::read_string(settings_file);
			toml::Reader reader(&wrapper, settings_file.string(), content);
			reader.read();
            LOG_INFO("Engine settings read succesfully");
		}
        engine = std::make_unique<Engine>(settings);

        // Настройка назначения клавиш
        std::filesystem::path controls_file = platform::get_controls_file();
        setup_bindings();
		if (std::filesystem::is_regular_file(controls_file)) {
			LOG_INFO("Loading bindings from '{}'", controls_file.string());
			std::string content = files::read_string(controls_file);
			load_controls(controls_file.string(), content);
            LOG_INFO("Bindings read succesfully");
		}

        engine->mainloop(); // Запуск основного цикла

        // Запись настроек движка в файл
        LOG_INFO("Write engine settings to '{}'", settings_file.string());
		files::write_string(settings_file, wrapper.write());
        LOG_INFO("Engine settings are written");

        // Запись назначения клавиш в файл
        LOG_INFO("Write bindings to '{}'", controls_file.string());
		files::write_string(controls_file, write_controls());
        LOG_INFO("Bindings are written");
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred\n{}", err.what());
    }

    Logger::getInstance().flush();

    return 0;
}
