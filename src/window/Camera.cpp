#include "Camera.h"
#include "Window.h"

#include <glm/ext.hpp>

// Конструктор камеры
Camera::Camera(glm::vec3 position, float fov) : position(position), fov(fov), rotation(1.0f){
    updateVectors();
}

// Обновляет векторы направления камеры на основе текущей матрицы вращения.
void Camera::updateVectors() {
    front = glm::vec3(rotation * glm::vec4(0, 0, -1, 1));
    right = glm::vec3(rotation * glm::vec4(1, 0, 0, 1));
    up = glm::vec3(rotation * glm::vec4(0, 1, 0, 1));
}

// Поворачивает камеру на заданные углы вокруг осей.
void Camera::rotate(float x, float y, float z) {
    // Вращение вокруг оси Z
    if (z != 0.0f) {
        rotation = glm::rotate(rotation, z, glm::vec3(0, 0, 1));
    }

    // Вращение вокруг оси Y
    // Поворачивает камеру влево/вправо
    if (y != 0.0f) {
        rotation = glm::rotate(rotation, y, glm::vec3(0, 1, 0));
    }

    // Вращение вокруг оси X
    // Наклоняет камеру вверх/вниз
    if (x != 0.0f) {
        rotation = glm::rotate(rotation, x, glm::vec3(1, 0, 0));
    }

    updateVectors();
}

// Возвращает матрицу проекции камеры.
// Матрица проекции преобразует координаты из пространства камеры в нормализованные координаты устройства (NDC).
glm::mat4 Camera::getProjection() {
    float aspect = (float)Window::width / (float)Window::height; // Вычисление соотношения сторон из текущих размеров окна
    return glm::perspective(fov, aspect, 0.1f, 100.0f);
}

// Возвращает матрицу вида камеры.
// Матрица вида преобразует мировые координаты в координаты камеры.
glm::mat4 Camera::getView() {
    return glm::lookAt(position, position + front, up);
}
