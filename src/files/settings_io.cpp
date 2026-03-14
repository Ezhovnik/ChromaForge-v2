#include "settings_io.h"

#include <memory>

#include "../window/Events.h"
#include "../window/input.h"
#include "../coders/json.h"
#include "../data/dynamic.h"
#include "../coders/toml.h"
#include "../logger/Logger.h"

toml::Wrapper* create_wrapper(EngineSettings& settings) {
	std::unique_ptr<toml::Wrapper> wrapper (new toml::Wrapper());

	toml::Section& display = wrapper->add("display");
	display.add("width", &settings.display.width);
	display.add("height", &settings.display.height);
	display.add("samples", &settings.display.samples);
	display.add("swap-interval", &settings.display.swapInterval);
	display.add("fullscreen", &settings.display.fullscreen);

	toml::Section& chunks = wrapper->add("chunks");
	chunks.add("load-distance", &settings.chunks.loadDistance);
	chunks.add("load-speed", &settings.chunks.loadSpeed);
	chunks.add("padding", &settings.chunks.padding);

    toml::Section& camera = wrapper->add("camera");
	camera.add("fov-events", &settings.camera.fovEvents);
	camera.add("shaking", &settings.camera.shaking);
	camera.add("fov", &settings.camera.fov);
	camera.add("sensitivity", &settings.camera.sensitivity);

	toml::Section& graphics = wrapper->add("graphics");
	graphics.add("fog-curve", &settings.graphics.fogCurve);
	graphics.add("backlight", &settings.graphics.backlight);
	graphics.add("frustum-culling", &settings.graphics.frustumCulling);
	graphics.add("skybox-resolution", &settings.graphics.skyboxResolution);
	graphics.add("gamma", &settings.graphics.gamma);

    toml::Section& debug = wrapper->add("debug");
	debug.add("generator-test-mode", &settings.debug.generatorTestMode);
	debug.add("do-write-lights", &settings.debug.doWriteLights);

	toml::Section& ui = wrapper->add("ui");
    ui.add("language", &settings.ui.language);

	toml::Section& audio = wrapper->add("audio");
    audio.add("enabled", &settings.audio.enabled);
    audio.add("volume-master", &settings.audio.volumeMaster);
    audio.add("volume-regular", &settings.audio.volumeRegular);
    audio.add("volume-ui", &settings.audio.volumeUI);
    audio.add("volume-ambient", &settings.audio.volumeAmbient);
    audio.add("volume-music", &settings.audio.volumeMusic);

	return wrapper.release();
}

std::string write_controls() {
	dynamic::Map obj;
	for (auto& [name, binding] : Events::bindings) {
		auto& jentry = obj.putMap(name);
		switch (binding.type) {
			case inputType::keyboard: jentry.put("type", "keyboard"); break;
			case inputType::mouse: jentry.put("type", "mouse"); break;
			default:
				LOG_ERROR("Unsupported control type {}", (int)binding.type);			
				throw std::runtime_error("Unsupported control type");
		}
		jentry.put("code", binding.code);
	}
	return json::stringify(&obj, true, "  ");
}

void load_controls(std::string filename, std::string source) {
	auto obj = json::parse(filename, source);
	for (auto& [name, binding] : Events::bindings) {
		auto jentry = obj->map(name);
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
