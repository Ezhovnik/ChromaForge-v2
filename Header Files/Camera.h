#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <../include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <../include/glm/glm.hpp>
#include <../include/glm/gtc/matrix_transform.hpp>
#include <../include/glm/gtc/type_ptr.hpp>
#include <../include/glm/gtx/rotate_vector.hpp>
#include <../include/glm/gtx/vector_angle.hpp>

#include "shaderClass.h"

class Camera {
    public:
        // Сохраняем основные векторы камеры
        glm::vec3 Position;
        glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 cameraMatrix = glm::mat4(1.0f);

        bool firstClick = true; // Предотвращает скачкообразное перемещение камеры при первом щелчке левой кнопкой мыши

        int width, height; // Переменные для хранения ширины и высоты окна

        // Отрегулируем скорость камеры и её чувствительность при осмотре
        float speed = 0.001f;
        float sensitivity = 100.0f;

        Camera(int width, int height, glm::vec3 position); // Конструктор камеры для установки начальных значений

        void updateMatrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader); // Обновляем и экспортируем матрицу камеры в вершинный шейдер
        void Matrix(Shader& shader, const char* uniform);
        void Inputs(GLFWwindow* window); // Управляем камерой
};

#endif