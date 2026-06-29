#include <window/Events.h>

#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <debug/Logger.h>
#include <data/dv.h>
#include <util/stringutil.h>

inline constexpr short _MOUSE_KEYS_OFFSET = 1024;

namespace {
    bool keys[KEYS_BUFFER_SIZE] = {};
    uint frames[KEYS_BUFFER_SIZE] = {};
    uint current_frame = 0;
    bool cursor_drag = false;
    bool cursor_locked = false;
    std::unordered_map<keycode, util::HandlersList<>> key_callbacks;
}

// Переменные для отслеживания позиции мыши
glm::vec2 Events::delta = {};
glm::vec2 Events::cursor = {};

int Events::scroll = 0;

std::vector<uint> Events::codepoints;
std::vector<keycode> Events::pressedKeys;
Bindings Events::bindings {};

int Events::getScroll() {
    return scroll;
}

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
    return Events::isPressed(keycode) && frames[keycode] == current_frame;
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
    cursor_locked = !cursor_locked;
    Window::setCursorMode(
        cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
    );
}

void Events::bind(const std::string& name, inputType type, keycode code) {
	bind(name, type, static_cast<int>(code));
}

void Events::bind(const std::string& name, inputType type, mousecode code) {
	bind(name, type, static_cast<int>(code));
}

void Events::bind(const std::string& name, inputType type, int code) {
	bindings.bind(name, type, code);
}

void Events::rebind(const std::string& name, inputType type, int code) {
    requireBinding(name) = Binding(type, code);
}

bool Events::isActive(const std::string& name) {
	return bindings.isActive(name);
}

bool Events::justActive(const std::string& name) {
	return bindings.justActive(name);
}

// Обработка событий текущего кадра
void Events::pollEvents() {
    current_frame++;
    delta.x = 0.0f;
    delta.y = 0.0f;
    codepoints.clear();
    pressedKeys.clear();
	scroll = 0;

    glfwPollEvents();

    for (auto& [name, binding] : bindings.getAll()) {
        if (!binding.enabled) {
            binding.state = false;
            continue;
        }
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
    ::keys[key] = b;
    ::frames[key] = current_frame;
    if (b) {
        const auto& callbacks = ::key_callbacks.find(static_cast<keycode>(key));
        if (callbacks != ::key_callbacks.end()) {
            callbacks->second.notify();
        }
    }
}

void Events::setButton(int button, bool b) {
    setKey(_MOUSE_KEYS_OFFSET + button, b);
}

void Events::setPosition(float xpos, float ypos) {
    if (::cursor_drag) {
        Events::delta.x += xpos - Events::cursor.x;
        Events::delta.y += ypos - Events::cursor.y;
    } else {
        ::cursor_drag = true;
	}

    Events::cursor.x = xpos;
    Events::cursor.y = ypos;
}

observer_handler Events::addKeyCallback(keycode key, KeyCallback callback) {
    return ::key_callbacks[key].add(std::move(callback));
}

Binding* Events::getBinding(const std::string& name) {
    return bindings.get(name);
}

Binding& Events::requireBinding(const std::string& name) {
    if (const auto found = getBinding(name)) {
        return *found;
    }
    LOG_ERROR("Binding '{}' does not exist");
    throw std::runtime_error("Binding '" + name + "' does not exist");
}

bool Events::isCursorLocked() {
    return cursor_locked;
}
