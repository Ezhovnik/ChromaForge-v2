#include "Camera.h"
#include "Window.h"

#include <glm/ext.hpp>

// Конструктор камеры
Camera::Camera(glm::vec3 position, float fov) : position(position), fov(fov), rotation(1.0f), zoom(1.0f){
    updateVectors();
}

// Обновляет векторы направления камеры на основе текущей матрицы вращения.
void Camera::updateVectors() {
    front = glm::vec3(rotation * glm::vec4(0,0,-1,1));
	right = glm::vec3(rotation * glm::vec4(1,0,0,1));
	up = glm::vec3(rotation * glm::vec4(0,1,0,1));

	dir = glm::vec3(rotation * glm::vec4(0,0,-1,1));
	dir.y = 0;

	float len = glm::length(dir);
	if (len > 0.0f){
		dir.x /= len;
		dir.z /= len;
	}
}

// Поворачивает камеру на заданные углы вокруг осей.
void Camera::rotate(float x, float y, float z) {
    // Вращение вокруг оси Z
    rotation = glm::rotate(rotation, z, glm::vec3(0, 0, 1));
    rotation = glm::rotate(rotation, y, glm::vec3(0, 1, 0));
    rotation = glm::rotate(rotation, x, glm::vec3(1, 0, 0));

    updateVectors();
}

// Возвращает матрицу проекции камеры.
// Матрица проекции преобразует координаты из пространства камеры в нормализованные координаты устройства (NDC).
glm::mat4 Camera::getProjection() {
    float aspect = this->aspect;
	if (aspect == 0.0f) aspect = (float)Window::width / (float)Window::height;
	
	if (perspective) return glm::perspective(fov*zoom, aspect, 0.05f, 1500.0f);
	else if (flipped) return glm::ortho(0.0f, fov*aspect, fov, 0.0f);
    else return glm::ortho(0.0f, fov*aspect, 0.0f, fov);
}

// Возвращает матрицу вида камеры.
// Матрица вида преобразует мировые координаты в координаты камеры.
glm::mat4 Camera::getView(bool position_flag) {
	glm::vec3 position = this->position;
	if (!position_flag) position = glm::vec3(0.0f);

    if (perspective) return glm::lookAt(position, position + front, up);
	else return glm::translate(glm::mat4(1.0f), position);
}

glm::mat4 Camera::getProjView() {
    return getProjection() * getView();
}
