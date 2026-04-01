#include "Events.h"

#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../debug/Logger.h"
#include "../data/dynamic.h"
#include "../coders/json.h"
#include "../util/stringutil.h"
#include "../coders/toml.h"

inline constexpr short _MOUSE_KEYS_OFFSET = 1024;

// Статические массивы для хранения состояние клавиш и кнопок мыши
bool Events::keys[KEYS_BUFFER_SIZE] = {}; // Хранит текущее состояние (нажата / не нажата)
uint Events::frames[KEYS_BUFFER_SIZE] = {}; // Храние номер кадра, в котором было последнее изменение состояния
uint Events::currentFrame = 0; // Номер текущего кадра

// Переменные для отслеживания позиции мыши
glm::vec2 Events::delta = {};
glm::vec2 Events::cursor = {};

// Флаги для упраления состоянием курсора
bool Events::_cursor_locked = false; // Режим захвата курсора
bool Events::cursor_drag = false; // Начал ли пользователь движение мышью

int Events::scroll = 0;

std::vector<uint> Events::codepoints;
std::vector<keycode> Events::pressedKeys;
std::unordered_map<std::string, Binding> Events::bindings;

bool Events::isPressed(keycode code) {
	return isPressed(static_cast<int>(code));
}

// Проверяет, нажата ли клавиша в данный момент
bool Events::isPressed(int keycode) {
    if (keycode < 0 || keycode  >= KEYS_BUFFER_SIZE) return false;

    return keys[keycode];
}

bool Events::justPressed(keycode code) {
	return justPressed(static_cast<int>(code));
}

// Проверяет, была ли клавиша нажата именно в текущем кадре 
bool Events::justPressed(int keycode) {
    return Events::isPressed(keycode) && frames[keycode] == currentFrame;
}

bool Events::isClicked(mousecode button) {
	return isClicked(static_cast<int>(button));
}

// Проверяет, нажата ли кнопка мыши в данный момент
bool Events::isClicked(int button) {
    return Events::isPressed(_MOUSE_KEYS_OFFSET + button);
}

bool Events::justClicked(mousecode button) {
	return justClicked(static_cast<int>(button));
}

// Проверяет, была ли кнопка мыши нажата именно в текущем кадре
bool Events::justClicked(int button) {
    return Events::justPressed(_MOUSE_KEYS_OFFSET + button);
}

// Переключает режим курсора между нормальным и заблокированным состоянием
void Events::toggleCursor() {
	cursor_drag = false;
    _cursor_locked = !_cursor_locked;
    Window::setCursorMode(_cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Events::bind(const std::string& name, inputType type, keycode code) {
	bind(name, type, static_cast<int>(code));
}

void Events::bind(const std::string& name, inputType type, mousecode code) {
	bind(name, type, static_cast<int>(code));
}

void Events::bind(const std::string& name, inputType type, int code) {
	bindings.emplace(name, Binding(type, code));
}

void Events::rebind(const std::string& name, inputType type, int code) {
    bindings[name] = Binding(type, code);
}

bool Events::isActive(const std::string& name) {
	const auto& found = bindings.find(name);
	if (found == bindings.end()) {
        LOG_WARN("Binding {} not found", name);
		return false;
	}
	return found->second.isActive();
}

bool Events::justActive(const std::string& name) {
	const auto& found = bindings.find(name);
	if (found == bindings.end()) {
        LOG_WARN("Binding {} not found", name);
		return false;
	}
	return found->second.justActive();
}

// Обработка событий текущего кадра
void Events::pollEvents() {
    currentFrame++;
    delta.x = 0.0f;
    delta.y = 0.0f;
    codepoints.clear();
    pressedKeys.clear();
	scroll = 0;

    glfwPollEvents();

    for (auto& [name, binding] : bindings) {
		binding.justChange = false;

		bool newstate = false;
		switch (binding.type) {
			case inputType::keyboard: newstate = isPressed(binding.code); break;
			case inputType::mouse: newstate = isClicked(binding.code); break;
		}

		if (newstate) {
			if (!binding.state) {
				binding.state = true;
				binding.justChange = true;
                binding.onactived.notify();
			}
		} else {
			if (binding.state) {
				binding.state = false;
				binding.justChange = true;
			}
		}
	}
}

void Events::setKey(int key, bool b) {
    Events::keys[key] = b;
    Events::frames[key] = Events::currentFrame;
}

void Events::setButton(int button, bool b) {
    setKey(_MOUSE_KEYS_OFFSET + button, b);
}

void Events::setPosition(float xpos, float ypos) {
    if (Events::cursor_drag) {
        Events::delta.x += xpos - Events::cursor.x;
        Events::delta.y += ypos - Events::cursor.y;
    } else {
        Events::cursor_drag = true;
	}

    Events::cursor.x = xpos;
    Events::cursor.y = ypos;
}

std::string Events::writeBindings() {
    dynamic::Map obj;
    for (auto& entry : Events::bindings) {
        const auto& binding = entry.second;

        std::string value;
        switch (binding.type) {
            case inputType::keyboard: 
                value = "key:"+input_util::get_name(static_cast<keycode>(binding.code)); 
                break;
            case inputType::mouse: 
                value = "mouse:"+input_util::get_name(static_cast<mousecode>(binding.code));
                break;
            default:
				LOG_ERROR("Unsupported control type");
				throw std::runtime_error("Unsupported control type");
        }
        obj.put(entry.first, value);
    }
    return toml::stringify(obj);
}

void Events::loadBindings(const std::string& filename, const std::string& source) {
    auto map = toml::parse(filename, source);
    for (auto& entry : map->values) {
        if (auto value = std::get_if<std::string>(&entry.second)) {
            auto [prefix, codename] = util::split_at(*value, ':');
            inputType type;
            int code;
            if (prefix == "key") {
                type = inputType::keyboard;
                code = static_cast<int>(input_util::keycode_from(codename));
            } else if (prefix == "mouse") {
                type = inputType::mouse;
                code = static_cast<int>(input_util::mousecode_from(codename));
            } else {
                LOG_ERROR("Unknown input type: {} (binding {})", prefix, util::quote(entry.first));
                continue;
            }
            Events::bind(entry.first, type, code);
        } else {
            LOG_ERROR("Invalid binding entry: {}", entry.first);
        }
    }
}
