#include "Window.h"

#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Events.h"
#include "../logger/Logger.h"
#include "../graphics/ImageData.h"

GLFWwindow* Window::window = nullptr; // Статическая переменная-член класса - указатель на окно GLFW
DisplaySettings* Window::settings = nullptr;
std::stack<glm::vec4> Window::scissorStack;
glm::vec4 Window::scissorArea;
uint Window::width = 0;
uint Window::height = 0;
int Window::posX = 0;
int Window::posY = 0;

const char* glfwErrorName(int error) {
	switch (error) {
		case GLFW_NO_ERROR: return "no error";
		case GLFW_NOT_INITIALIZED: return "not initialized";
		case GLFW_NO_CURRENT_CONTEXT: return "no current context";
		case GLFW_INVALID_ENUM: return "invalid enum";
		case GLFW_INVALID_VALUE: return "invalid value";
		case GLFW_OUT_OF_MEMORY: return "out of memory";
		case GLFW_API_UNAVAILABLE: return "api unavailable";
		case GLFW_VERSION_UNAVAILABLE: return "version unavailable";
		case GLFW_PLATFORM_ERROR: return "platform error";
		case GLFW_FORMAT_UNAVAILABLE: return "format unavailable";
		case GLFW_NO_WINDOW_CONTEXT: return "no window context";
		default: return "unknown error";
	}
}

void error_callback(int error, const char* description) {
    std::stringstream ss;
    ss << "GLFW error [0x" << std::hex << error << "]: ";
    ss << glfwErrorName(error);
    if (description) ss << description;

    LOG_ERROR("{}", ss.str());
    Logger::getInstance().flush();
}

// Callback-функция для обработки движения мыши
void cursor_position_callback(GLFWwindow*, double x_pos, double y_pos) {
    if (Events::_cursor_locked) {
        // В режиме захвата курсора накапливаем изменения позиции
        Events::deltaX += x_pos - Events::x;
        Events::deltaY += y_pos - Events::y;
    } else {
        // В обычном режиме просто отмечаем, что курсор начал движение
        Events::_cursor_started = true;
    }
    // Обновляем текущие координаты
    Events::x = x_pos;
    Events::y = y_pos;
}

// Callback-функция для обработки нажатий кнопок мыши
void mouse_button_callback(GLFWwindow*, int button, int action, int mode) {
    if (button < 0 || button >= _MAX_MOUSE_BUTTONS) return;

    int index = _MOUSE_KEYS_OFFSET + button;

    if (action == GLFW_PRESS) {
        Events::_keys[index] = true;
        Events::_frames[index] = Events::_current;
    } else if (action == GLFW_RELEASE) {
        Events::_keys[index] = false;
        Events::_frames[index] = Events::_current;
    }
}

// Callback-функция для обработки нажатий клавиш клавиатуры
void key_callback(GLFWwindow*, int key, int scancode, int action, int mode) {
    if (key < 0 || key >= _MOUSE_KEYS_OFFSET) return;

    if (action == GLFW_PRESS) {
        Events::_keys[key] = true;
        Events::_frames[key] = Events::_current;
        Events::pressedKeys.push_back(key);
    } else if (action == GLFW_RELEASE) {
        Events::_keys[key] = false;
        Events::_frames[key] = Events::_current;
    } else if (action == GLFW_REPEAT) {
        Events::pressedKeys.push_back(key);
    }
}

bool Window::isMaximized() {
	return glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
}

bool Window::isFocused() {
	return glfwGetWindowAttrib(window, GLFW_FOCUSED);
}

// Callback-функция для обработки изменения размера окна
void window_size_callback(GLFWwindow*, int width, int height) {
    if (Window::isFocused() && width && height) {
        glViewport(0, 0, width, height); // Обновляем область отображения при изменении размера окна
        
        // Обновляем размеры окна у объекта окна
        Window::width = width;
        Window::height = height;
    }

    if (!Window::isMaximized()) {
		Window::getDisplaySettings()->width = width;
		Window::getDisplaySettings()->height = height;
	}

    Window::resetScissor();
}

void character_callback(GLFWwindow*, uint codepoint){
	Events::codepoints.push_back(codepoint);
}

void scroll_callback(GLFWwindow*, double xoffset, double yoffset) {
    Events::scroll += yoffset;
}

// Инициализация окна и OpenGL контекста
bool Window::initialize(DisplaySettings& settings) {
    if (!glfwInit()){ // Инициализация GLFW
        LOG_CRITICAL("GLFW initialization failed");
        return false;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    #else
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    #endif

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    LOG_DEBUG("GLFW version: {}.{}.{}", major, minor, rev);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Разрешаем изменять размер окна
    glfwWindowHint(GLFW_SAMPLES, settings.samples);

    // Создание окна GLFW
    window = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        LOG_CRITICAL("Failed to create GLFW Window");
        glfwTerminate();
        return false;
    }

    // Установка созданного окна как текущего контекста OpenGL
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        LOG_CRITICAL("Failed to initialize GLEW {}", reinterpret_cast<const char*>(glewGetErrorString(glewError)));
        return false;
    }
     // Установка области отображения (viewport) - вся область окна
    glViewport(0, 0, settings.width, settings.height);

    glClearColor(0.0f, 0.0f, 0.0f, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Window::settings = &settings;
    Window::width = settings.width;
    Window::height = settings.height;

    Events::initialize();

    // Устанавливаем callback-функции GLFW
    glfwSetKeyCallback(window, key_callback); // Клавиатура
    glfwSetMouseButtonCallback(window, mouse_button_callback); // Нажатие кнопки мыши
    glfwSetCursorPosCallback(window, cursor_position_callback); // Движение мыши
    glfwSetWindowSizeCallback(window, window_size_callback); // Изменение размера окна
    glfwSetCharCallback(window, character_callback);
    glfwSetScrollCallback(window, scroll_callback);

	if (settings.fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }

    glfwSwapInterval(settings.swapInterval);

    const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
    LOG_DEBUG("GL Vendor: {}", (char*)vendor);
    LOG_DEBUG("GL Renderer: {}", (char*)renderer);

    LOG_INFO("Window initialized successfully: {}x{}", settings.width, settings.height);

    return true;
}

// Завершение работы окна и освобождение ресурсов GLFW
void Window::terminate() {
    Events::finalize();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::setCursorMode(int mode) {
    glfwSetInputMode(window, GLFW_CURSOR, mode);
}

void Window::resetScissor() {
	scissorArea = glm::vec4(0.0f, 0.0f, width, height);
	scissorStack = std::stack<glm::vec4>();
	glDisable(GL_SCISSOR_TEST);
}

void Window::pushScissor(glm::vec4 area) {
	if (scissorStack.empty()) glEnable(GL_SCISSOR_TEST);

	scissorStack.push(scissorArea);

    area.z += area.x;
    area.w += area.y;

	area.x = fmax(area.x, scissorArea.x);
	area.y = fmax(area.y, scissorArea.y);

	area.z = fmin(area.z, scissorArea.z);
	area.w = fmin(area.w, scissorArea.w);

	if (area.z < 0.0f || area.w < 0.0f) glScissor(0, 0, 0, 0);
	else glScissor(area.x, Window::height - area.w, std::max(0, int(area.z - area.x)), std::max(0, int(area.w - area.y)));

	scissorArea = area;
}

void Window::popScissor() {
	if (scissorStack.empty()) {
        LOG_WARN("Extra Window::popScissor call");
		return;
	}
	glm::vec4 area = scissorStack.top();
	scissorStack.pop();

	if (area.z < 0.0f || area.w < 0.0f) glScissor(0, 0, 0, 0);
	else glScissor(area.x, Window::height - area.w, std::max(0, int(area.z - area.x)), std::max(0, int(area.w - area.y)));

	if (scissorStack.empty()) glDisable(GL_SCISSOR_TEST);
	scissorArea = area;
}

void Window::viewport(int x, int y, int width, int height){
	glViewport(x, y, width, height);
}

// Проверка, должно ли окно закрыться
bool Window::isShouldClose() {
    return glfwWindowShouldClose(window);
}

// Установка флага закрытия окна
void Window::setShouldClose(bool flag) {
    glfwSetWindowShouldClose(window, flag);
}

void Window::swapInterval(int interval){
	glfwSwapInterval(interval);
}

// Обмен буферов
void Window::swapBuffers() {
    glfwSwapBuffers(window);
    Window::resetScissor();
}

double Window::time() {
	return glfwGetTime();
}

void Window::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::clearDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Window::setBgColor(glm::vec3 color) {
    glClearColor(color.r, color.g, color.b, 1.0f);
}

void Window::toggleFullscreen(){
	settings->fullscreen = !settings->fullscreen;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (Events::_cursor_locked) Events::toggleCursor();

	if (settings->fullscreen) {
        glfwGetWindowPos(window, &posX, &posY);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	} else {
		glfwSetWindowMonitor(window, nullptr, posX, posY, settings->width, settings->height, GLFW_DONT_CARE);
		glfwSetWindowAttrib(window, GLFW_MAXIMIZED, GLFW_FALSE);
	}

	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	Events::x = xPos;
	Events::y = yPos;
}

bool Window::isFullscreen() {
	return settings->fullscreen;
}

DisplaySettings* Window::getDisplaySettings() {
	return settings;
}

ImageData* Window::takeScreenshot() {
	ubyte* data = new ubyte[width * height * 4];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	return new ImageData(ImageFormat::rgba8888, width, height, data);
}

bool Window::tryToMaximize(GLFWwindow* window, GLFWmonitor* monitor) {
	glm::ivec4 windowFrame(0);
	glm::ivec4 workArea(0);
	glfwGetWindowFrameSize(window, &windowFrame.x, &windowFrame.y, &windowFrame.z, &windowFrame.w);
	glfwGetMonitorWorkarea(monitor, &workArea.x, &workArea.y, &workArea.z, &workArea.w);
	if (Window::width > (uint)workArea.z) Window::width = (uint)workArea.z;
	if (Window::height > (uint)workArea.w) Window::height = (uint)workArea.w;
	if (Window::width >= (uint)(workArea.z - (windowFrame.x + windowFrame.z)) &&
		Window::height >= (uint)(workArea.w - (windowFrame.y + windowFrame.w))) {
		glfwMaximizeWindow(window);
		return true;
	}
	glfwSetWindowSize(window, Window::width, Window::height);
	glfwSetWindowPos(window, workArea.x + (workArea.z - Window::width) / 2, workArea.y + (workArea.w - Window::height) / 2 + windowFrame.y / 2);
	return false;
}
