#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// Виртуальная камера в 3D пространстве
class Camera {
private:
    float fov = 1.0f;
    float ar = 0.0f;
public:
    glm::vec3 position {}; // Позиция камеры
    float zoom = 1.0f;

    // Векторы направления
    glm::vec3 front {}; // Указывает направление, в котором смотрит камера.
    glm::vec3 up {}; // Определяет ориентацию камеры относительно вертикали.
    glm::vec3 right {}; // Определяет горизонтальную ориентацию камеры.
    glm::vec3 dir {};

    bool flipped = false;
    bool perspective = true;

    float near = 0.05f;
    float far = 1500.0f;

    // Матрица вращения камеры
    glm::mat4 rotation {1.0f}; // Хранит текущую ориентацию камеры в пространстве.

    Camera() {
        updateVectors();
    }
    Camera(glm::vec3 position, float fov); // Конструктор

    void updateVectors(); // Обновляет векторы направления камеры на основе текущей матрицы вращения
    void rotate(float x, float y, float z); // Поворачивает камеру на заданные углы.

    glm::mat4 getProjection() const; // Возвращает матрицу проекции камеры.
    glm::mat4 getView(bool position_flag = true) const; // Возвращает матрицу вида камеры.
    glm::mat4 getProjView(bool position_flag = true) const;

    void setFov(float fov) {this->fov = fov;}
    float getFov() const {return fov;}

    float getAspectRatio() const;
    void setAspectRatio(float ar);
};
