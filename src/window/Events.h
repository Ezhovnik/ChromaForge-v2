#pragma once

#include <vector>
#include <string>
#include <vector>
#include <unordered_map>

#include <window/Window.h>
#include <window/input.h>
#include <typedefs.h>
#include <delegates.h>

inline constexpr short KEYS_BUFFER_SIZE = 1036;

namespace Events {
    extern int scroll;
    extern glm::vec2 delta;
    extern glm::vec2 cursor;
    extern std::vector<uint> codepoints;
    extern std::vector<keycode> pressedKeys;
    extern Bindings bindings;

    void pollEvents();

    int getScroll();

    bool isPressed(keycode keycode);
    bool isPressed(int keycode);
    bool justPressed(keycode keycode);
    bool justPressed(int keycode);

    bool isClicked(mousecode button);
    bool isClicked(int button);
    bool justClicked(mousecode button);
    bool justClicked(int button);

    void toggleCursor();

    Binding* getBinding(const std::string& name);
    Binding& requireBinding(const std::string& name);
    void bind(const std::string& name, inputType type, keycode code);
    void bind(const std::string& name, inputType type, mousecode code);
    void bind(const std::string& name, inputType type, int code);
    void rebind(const std::string& name, inputType type, int code);
    bool isActive(const std::string& name);
    bool justActive(const std::string& name);

    observer_handler addKeyCallback(keycode key, KeyCallback callback);

    void setKey(int key, bool b);
    void setButton(int button, bool b);

    void setPosition(float xpos, float ypos);

    bool isCursorLocked();
};
