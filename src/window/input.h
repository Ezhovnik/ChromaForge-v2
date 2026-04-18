#ifndef WINDOW_INPUT_H_
#define WINDOW_INPUT_H_

#include <string>

#include "util/RunnablesList.h"

/**
 * @brief Представляет собой значения кодов клавиш glfw3.
 */
enum class keycode : int {
    SPACE = 32,
    APOSTROPHE = 39,
    COMMA = 44,
    MINUS = 45,
    PERIOD = 46,
    SLASH = 47,
    NUM_0 = 48,
    NUM_1 = 49,
    NUM_2 = 50,
    NUM_3 = 51,
    NUM_4 = 52,
    NUM_5 = 53,
    NUM_6 = 54,
    NUM_7 = 55,
    NUM_8 = 56,
    NUM_9 = 57,
    SEMICOLON = 59,
    EQUAL = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LEFT_BRACKET = 91,
    BACKSLASH = 92,
    RIGHT_BRACKET = 93,
    GRAVE_ACCENT = 96,
    ESCAPE = 256,
    ENTER = 257,
    TAB = 258,
    BACKSPACE = 259,
    INSERT = 260,
    DEL = 261,
    LEFT = 263,
    RIGHT = 262,
    DOWN = 264,
    UP = 265,
    PAGE_UP = 266,
    PAGE_DOWN = 267,
    HOME = 268,
    END = 269,
    CAPS_LOCK = 280,
    NUM_LOCK = 282,
    PRINT_SCREEN = 283,
    PAUSE = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    LEFT_SHIFT = 340,
    LEFT_CONTROL = 341,
    LEFT_ALT = 342,
    LEFT_SUPER = 343,
    RIGHT_SHIFT = 344,
    RIGHT_CONTROL = 345,
    RIGHT_ALT = 346,
    RIGHT_SUPER = 347,
    MENU = 348,
    UNKNOWN = -1
};

/**
* @brief Представляет собой набор идентификаторов кнопок мыши glfw3. 
* @details Существует подмножество идентификаторов кнопок мыши glfw3.
*/
enum class mousecode : int {
	BUTTON_1 = 0, ///< Левая кнопка мыши
	BUTTON_2 = 1, ///< Правая кнопка мыши
	BUTTON_3 = 2, ///< Средняя кнопка мыши (колёсико)
    UNKNOWN = -1,
};

inline mousecode MOUSECODES_ALL[] {
	mousecode::BUTTON_1,
	mousecode::BUTTON_2,
	mousecode::BUTTON_3
};

namespace input_util {
	void initialize();

    keycode keycode_from(const std::string& name);
    mousecode mousecode_from(const std::string& name);

	/**
     * @brief Возвращает название клавиши по её коду.
     * @param code Код клавиши.
     */
	std::string to_string(keycode code);

    /**
     * @brief Возвращает название кнопки мыши по её коду.
     * @param code Код кнопки мыши.
     */
	std::string to_string(mousecode code);

    std::string get_name(keycode code);
    std::string get_name(mousecode code);
}

/**
 * @brief Тип устройства ввода.
 */
enum class inputType {
    keyboard, mouse
};

/**
 * @brief Представляет привязку действия к клавише или кнопке мыши.
 *
 * Хранит текущее состояние (нажата или нет) и флаг, было ли состояние изменено в текущем кадре.
 */
struct Binding {
	util::RunnablesList onactived;

    inputType type;
    int code;
    bool state = false;
    bool justChange = false;

	Binding() = default;
    Binding(inputType type, int code) : type(type), code(code) {}

    /**
     * @brief Проверяет, активно ли нажатие (кнопка удерживается).
     * @return true, если кнопка нажата.
     */
    bool isActive() {
        return state;
    }

    /**
     * @brief Проверяет, было ли только что совершено нажатие (в этом кадре).
     * @return true, если кнопка нажата и состояние изменилось в этом кадре.
     */
    bool justActive() {
        return state && justChange;
    }

    void reset(inputType, int);
	void reset(keycode);
	void reset(mousecode);

    /**
     * @brief Возвращает текстовое представление привязки (имя клавиши/кнопки).
     * @return Строка с именем.
     */
    inline std::string text() const {
        switch (type) {
            case inputType::keyboard: return input_util::to_string(static_cast<keycode>(code));
            case inputType::mouse: return input_util::to_string(static_cast<mousecode>(code));
        }
        return "<unknown input type>";
    }
};

#endif // WINDOW_INPUT_H_
