#include "Window.h"

#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Events.h"
#include "../debug/Logger.h"
#include "../graphics/core/ImageData.h"
#include "../graphics/core/Texture.h"
#include "../settings.h"
#include "../util/ObjectsKeeper.h"
#include "../constants.h"

GLFWwindow* Window::window = nullptr; // Статическая переменная-член класса - указатель на окно GLFW
DisplaySettings* Window::settings = nullptr;
std::stack<glm::vec4> Window::scissorStack;
glm::vec4 Window::scissorArea;
uint Window::width = 0;
uint Window::height = 0;
int Window::posX = 0;
int Window::posY = 0;
bool Window::fullscreen = false;
static util::ObjectsKeeper observers_keeper;

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
}

// Callback-функция для обработки движения мыши
void cursor_position_callback(GLFWwindow*, double x_pos, double y_pos) {
    Events::setPosition(x_pos, y_pos);
}

// Callback-функция для обработки нажатий кнопок мыши
void mouse_button_callback(GLFWwindow*, int button, int action, int mode) {
    Events::setButton(button, action == GLFW_PRESS);
}

// Callback-функция для обработки нажатий клавиш клавиатуры
void key_callback(GLFWwindow*, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_UNKNOWN) return;

    if (action == GLFW_PRESS) {
        Events::setKey(key, true);
        Events::pressedKeys.push_back(static_cast<keycode>(key));
    } else if (action == GLFW_RELEASE) {
        Events::setKey(key, false);
    } else if (action == GLFW_REPEAT) {
        Events::pressedKeys.push_back(static_cast<keycode>(key));
    }
}

bool Window::isMaximized() {
	return glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
}

bool Window::isFocused() {
	return glfwGetWindowAttrib(window, GLFW_FOCUSED);
}

bool Window::isIconified() {
    return glfwGetWindowAttrib(window, GLFW_ICONIFIED);
}

// Callback-функция для обработки изменения размера окна
void window_size_callback(GLFWwindow*, int width, int height) {
    if (width && height) {
		if (Window::isFocused()) {
			glViewport(0, 0, width, height);
			Window::width = width;
			Window::height = height;
		}

		if (!Window::isFullscreen() && !Window::isMaximized()) {
			Window::getDisplaySettings()->width.set(width);
			Window::getDisplaySettings()->height.set(height);
		}
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
bool Window::initialize(DisplaySettings* settings) {
    Window::settings = settings;
    Window::width = settings->width.get();
    Window::height = settings->height.get();

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
    glfwWindowHint(GLFW_SAMPLES, settings->samples.get());

    std::string title = "ChromaForge (v " + ENGINE_VERSION_STRING + ")";

    // Создание окна GLFW
    window = glfwCreateWindow(
        width, 
        height, 
        title.c_str(), 
        nullptr, 
        nullptr
    );
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
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint maxTextureSize[1]{static_cast<GLint>(Texture::MAX_RESOLUTION)};
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
    if (maxTextureSize[0] > 0) {
        Texture::MAX_RESOLUTION = maxTextureSize[0];
        LOG_INFO("Max texture size is {}", Texture::MAX_RESOLUTION);
    }

    // Устанавливаем callback-функции GLFW
    glfwSetKeyCallback(window, key_callback); // Клавиатура
    glfwSetMouseButtonCallback(window, mouse_button_callback); // Нажатие кнопки мыши
    glfwSetCursorPosCallback(window, cursor_position_callback); // Движение мыши
    glfwSetWindowSizeCallback(window, window_size_callback); // Изменение размера окна
    glfwSetCharCallback(window, character_callback);
    glfwSetScrollCallback(window, scroll_callback);

	observers_keeper = util::ObjectsKeeper();
    observers_keeper.keepAlive(settings->fullscreen.observe([=](bool value) {
        if (value != isFullscreen()) {
            toggleFullscreen();
        }
    }, true));
    glfwSwapInterval(settings->vsync.get());

    const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
    LOG_DEBUG("GL Vendor: {}", (char*)vendor);
    LOG_DEBUG("GL Renderer: {}", (char*)renderer);
    LOG_DEBUG("GLFW version: {}", glfwGetVersionString());

    input_util::initialize();

    LOG_INFO("Window initialized successfully: {}x{}", settings->width.get(), settings->height.get());

    return true;
}

// Завершение работы окна и освобождение ресурсов GLFW
void Window::terminate() {
    observers_keeper = util::ObjectsKeeper();
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

const char* Window::getClipboardText() {
    return glfwGetClipboardString(window);
}

void Window::setClipboardText(const char* text) {
    glfwSetClipboardString(window, text);
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

void Window::setBgColor(glm::vec4 color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void Window::toggleFullscreen(){
	fullscreen = !fullscreen;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (Events::_cursor_locked) Events::toggleCursor();

	if (fullscreen) {
        glfwGetWindowPos(window, &posX, &posY);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	} else {
		glfwSetWindowMonitor(window, nullptr, posX, posY, settings->width.get(), settings->height.get(), GLFW_DONT_CARE);
		glfwSetWindowAttrib(window, GLFW_MAXIMIZED, GLFW_FALSE);
	}

	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	Events::setPosition(xPos, yPos);
}

bool Window::isFullscreen() {
	return fullscreen;
}

DisplaySettings* Window::getDisplaySettings() {
	return settings;
}

std::unique_ptr<ImageData> Window::takeScreenshot() {
    auto data = std::make_unique<ubyte[]>(width * height * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
	return std::make_unique<ImageData>(
        ImageFormat::rgba8888, width, height, data.release()
    );
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
