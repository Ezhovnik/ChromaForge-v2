#include <window/Camera.h>

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
glm::mat4 Camera::getProjection() const {
	if (perspective) {
		return glm::perspective(fov * zoom, ar, near, far);
	} else if (flipped) {
		return glm::ortho(0.0f, fov * ar, fov, 0.0f, near, far);
	} else {
		return glm::ortho(0.0f, fov * ar, 0.0f, fov, near, far);
	}
}

// Возвращает матрицу вида камеры.
// Матрица вида преобразует мировые координаты в координаты камеры.
glm::mat4 Camera::getView(bool position_flag) const {
	glm::vec3 camera_pos = this->position;
	if (!position_flag) camera_pos = glm::vec3(0.0f);

    if (perspective) {
		return glm::lookAt(camera_pos, camera_pos + front, up);
	} else {
		return glm::lookAt(camera_pos, camera_pos + front, up);
	}
}

glm::mat4 Camera::getProjView(bool position_flag) const {
    return getProjection() * getView(position_flag);
}

float Camera::getAspectRatio() const {
    return ar;
}

void Camera::setAspectRatio(float ar) {
    this->ar = ar;
}
