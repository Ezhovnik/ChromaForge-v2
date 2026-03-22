#include "settings_io.h"

#include <memory>

#include "../window/Events.h"
#include "../window/input.h"
#include "../coders/json.h"
#include "../coders/toml.h"
#include "../debug/Logger.h"

SettingsHandler::SettingsHandler(EngineSettings& settings) {
    map.emplace("audio.volume-master", &settings.audio.volumeMaster);
    map.emplace("audio.volume-regular", &settings.audio.volumeRegular);
    map.emplace("audio.volume-ui", &settings.audio.volumeUI);
    map.emplace("audio.volume-ambient", &settings.audio.volumeAmbient);
    map.emplace("audio.volume-music", &settings.audio.volumeMusic);

	map.emplace("display.vsync", &settings.display.vsync);

    map.emplace("camera.sensitivity", &settings.camera.sensitivity);
	map.emplace("camera.fov", &settings.camera.fov);
	map.emplace("camera.shaking", &settings.camera.shaking);

    map.emplace("chunks.load-distance", &settings.chunks.loadDistance);
    map.emplace("chunks.load-speed", &settings.chunks.loadSpeed);

    map.emplace("graphics.fog-curve", &settings.graphics.fogCurve);
	map.emplace("graphics.backlight", &settings.graphics.backlight);
}

std::unique_ptr<dynamic::Value> SettingsHandler::getValue(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
		LOG_ERROR("Setting '{}' does not exist", name);
        throw std::runtime_error("Setting '" + name + "' does not exist");
    }
    auto setting = found->second;
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        return dynamic::Value::of((number_t)number->get());
	} else if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        return dynamic::Value::of((integer_t)integer->get());
	} else if (auto flag = dynamic_cast<BoolSetting*>(setting)) {
        return dynamic::Value::boolean(flag->get());
    } else {
		LOG_ERROR("Type is not implemented for '{}'", name);
        throw std::runtime_error("Type is not implemented for '" + name + "'");
    }
}

template<class T>
static void set_numeric_value(T* setting, const dynamic::Value& value) {
    switch (value.type) {
        case dynamic::ValueType::Integer:
            setting->set(std::get<integer_t>(value.value));
            break;
        case dynamic::ValueType::Number:
            setting->set(std::get<number_t>(value.value));
            break;
        case dynamic::ValueType::Boolean:
            setting->set(std::get<bool>(value.value));
            break;
        default:
			LOG_ERROR("Type error, numeric value expected");
            throw std::runtime_error("Type error, numeric value expected");
    }
}

Setting* SettingsHandler::getSetting(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
		LOG_ERROR("Setting '{}' does not exist", name);
        throw std::runtime_error("setting '" + name + "' does not exist");
    }
    return found->second;
}

void SettingsHandler::setValue(const std::string& name, const dynamic::Value& value) {
    auto found = map.find(name);
    if (found == map.end()) {
		LOG_ERROR("Setting '{}' does not exist", name);
        throw std::runtime_error("Setting '" + name + "' does not exist");
    }
    auto setting = found->second;
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        set_numeric_value(number, value);
	} else if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
		set_numeric_value(integer, value);
    } else if (auto flag = dynamic_cast<BoolSetting*>(setting)) {
        set_numeric_value(flag, value);
    } else {
		LOG_ERROR("Type is not implemented - setting '{}'", name);
        throw std::runtime_error("Type is not implement - setting '" + name + "'");
    }
}

std::string SettingsHandler::toString(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
		LOG_ERROR("Setting '{}' does not exist", name);
        throw std::runtime_error("Setting '" + name + "' does not exist");
    }
    auto setting = found->second;
    return setting->toString();
}

toml::Wrapper* create_wrapper(EngineSettings& settings) {
	auto wrapper = std::make_unique<toml::Wrapper>();

	toml::Section& display = wrapper->add("display");
	display.add("width", &settings.display.width);
	display.add("height", &settings.display.height);
	display.add("samples", &settings.display.samples);
	display.add("vsync", &*settings.display.vsync);
	display.add("fullscreen", &settings.display.fullscreen);

	toml::Section& chunks = wrapper->add("chunks");
	chunks.add("load-distance", &*settings.chunks.loadDistance);
	chunks.add("load-speed", &*settings.chunks.loadSpeed);
	chunks.add("padding", &*settings.chunks.padding);

    toml::Section& camera = wrapper->add("camera");
	camera.add("fov-events", &settings.camera.fovEvents);
	camera.add("shaking", &*settings.camera.shaking);
	camera.add("fov", &*settings.camera.fov);
	camera.add("sensitivity", &*settings.camera.sensitivity);

	toml::Section& graphics = wrapper->add("graphics");
	graphics.add("fog-curve", &*settings.graphics.fogCurve);
	graphics.add("backlight", &*settings.graphics.backlight);
	graphics.add("frustum-culling", &settings.graphics.frustumCulling);
	graphics.add("skybox-resolution", &settings.graphics.skyboxResolution);
	graphics.add("gamma", &settings.graphics.gamma);

    toml::Section& debug = wrapper->add("debug");
	debug.add("generator-test-mode", &settings.debug.generatorTestMode);
	debug.add("do-write-lights", &settings.debug.doWriteLights);

	toml::Section& ui = wrapper->add("ui");
	ui.add("world-preview-size", &*settings.ui.worldPreviewSize);
    ui.add("language", &settings.ui.language);

	toml::Section& audio = wrapper->add("audio");
    audio.add("enabled", &settings.audio.enabled);
    audio.add("volume-master", &*settings.audio.volumeMaster);
    audio.add("volume-regular", &*settings.audio.volumeRegular);
    audio.add("volume-ui", &*settings.audio.volumeUI);
    audio.add("volume-ambient", &*settings.audio.volumeAmbient);
    audio.add("volume-music", &*settings.audio.volumeMusic);

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
