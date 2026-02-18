#ifndef WINDOW_CAMERA_H_
#define WINDOW_CAMERA_H_

#include <glm/glm.hpp>

// Виртуальная камера в 3D пространстве
class Camera {
    float fov;

    // Обновляет векторы направления камеры на основе текущей матрицы вращения
    void updateVectors(); 
public:
    glm::vec3 position; // Позиция камеры
    float zoom;

    // Векторы направления
    glm::vec3 front; // Указывает направление, в котором смотрит камера.
    glm::vec3 up; // Определяет ориентацию камеры относительно вертикали.
    glm::vec3 right; // Определяет горизонтальную ориентацию камеры.
    glm::vec3 dir;

    float aspect = 0.0f;

    bool flipped = false;
    bool perspective = true;

    // Матрица вращения камеры
    glm::mat4 rotation; // Хранит текущую ориентацию камеры в пространстве.

    Camera(glm::vec3 position, float fov); // Конструктор

    void rotate(float x, float y, float z); // Поворачивает камеру на заданные углы.

    glm::mat4 getProjection(); // Возвращает матрицу проекции камеры.
    glm::mat4 getView(bool position_flag = true); // Возвращает матрицу вида камеры.
    glm::mat4 getProjView();

    void setFov(float fov) {this->fov = fov;}
    float getFov() const {return fov;}
};

#endif // WINDOW_CAMERA_H
