#include <window/Camera.h>
#include <window/Window.h>

#include <glm/ext.hpp>

// Конструктор камеры
Camera::Camera(glm::vec3 position, float fov) : position(position), fov(fov) {
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
    rotation = glm::rotate(rotation, y, glm::vec3(0, 1, 0));
    rotation = glm::rotate(rotation, x, glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, z, glm::vec3(0, 0, 1));

    updateVectors();
}

// Возвращает матрицу проекции камеры.
// Матрица проекции преобразует координаты из пространства камеры в нормализованные координаты устройства (NDC).
glm::mat4 Camera::getProjection() {
    constexpr float epsilon = 1e-6f; // 0.000001
    float aspect_ratio = this->aspect;
    if (std::fabs(aspect_ratio) < epsilon) {
        aspect_ratio = (float)Window::width / (float)Window::height;
	}

	if (perspective) {
		return glm::perspective(fov * zoom, aspect_ratio, 0.05f, 1500.0f);
	} else if (flipped) {
		return glm::ortho(0.0f, fov * aspect_ratio, fov, 0.0f);
	} else {
		return glm::ortho(0.0f, fov * aspect_ratio, 0.0f, fov);
	}
}

// Возвращает матрицу вида камеры.
// Матрица вида преобразует мировые координаты в координаты камеры.
glm::mat4 Camera::getView(bool position_flag) {
	glm::vec3 camera_pos = this->position;
	if (!position_flag) camera_pos = glm::vec3(0.0f);

    if (perspective) return glm::lookAt(camera_pos, camera_pos + front, up);
	else return glm::translate(glm::mat4(1.0f), camera_pos);
}

glm::mat4 Camera::getProjView(bool position_flag) {
    return getProjection() * getView(position_flag);
}
