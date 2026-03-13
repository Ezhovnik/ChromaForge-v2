#include "Events.h"

#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../logger/Logger.h"

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

void Events::bind(std::string name, inputType type, keycode code) {
	bind(name, type, static_cast<int>(code));
}

void Events::bind(std::string name, inputType type, mousecode code) {
	bind(name, type, static_cast<int>(code));
}

void Events::bind(std::string name, inputType type, int code) {
	bindings[name] = {type, code, false, false};
}

bool Events::isActive(std::string name) {
	const auto& found = bindings.find(name);
	if (found == bindings.end()) {
        LOG_WARN("Binding {} not found", name);
		return false;
	}
	return found->second.isActive();
}

bool Events::justActive(std::string name) {
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
