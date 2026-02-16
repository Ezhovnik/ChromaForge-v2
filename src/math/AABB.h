#ifndef MATHS_AABB_H_
#define MATHS_AABB_H_

#include <glm/glm.hpp>

// Структура, представляющая выровненный по осям ограничивающий параллелепипед (AABB).
struct AABB {
	glm::vec3 a {0.0f}; // Первая точка параллелепипеда
    glm::vec3 b {1.0f}; // Вторая точка параллелепипеда

    inline glm::vec3 min() const {return glm::min(a, b);} // Минимальная координата по каждой оси
    inline glm::vec3 max() const {return glm::max(a, b);} // Максимальная координата по каждой оси

    // Размеры параллелепипеда по каждой оси
    inline glm::vec3 size() const {
        return glm::vec3(
            fabs(b.x - a.x),
            fabs(b.y - a.y),
            fabs(b.z - a.z)
        );
    }

    // Центр параллелепипеда
    inline glm::vec3 center() const {return (a + b) * 0.5f;}

    /* Масштабирует параллелепипед относительно центра. 
    После масштабирования центр остаётся на месте*/
    inline void scale(const glm::vec3 mul) {
        glm::vec3 center = (a + b) * 0.5f;
        a = (a - center) * mul + center;
        b = (b - center) * mul + center;
    }

    // Масштабирует параллелепипед относительно заданной точки в относительных координатах.
    inline void scale(const glm::vec3 mul, const glm::vec3 orig) {
        glm::vec3 beg = min();
        glm::vec3 end = max();
        glm::vec3 center = glm::mix(beg, end, orig);
        a = (a - center) * mul + center;
        b = (b - center) * mul + center;
    }

    // Проверяет, находится ли точка строго внутри параллелепипеда (включая левые и нижние грани, исключая правые и верхние).
    inline bool inside(const glm::vec3 pos) const {
        const glm::vec3 p = min();
        const glm::vec3 s = size();
        return !(pos.x < p.x || pos.y < p.y || pos.z < p.z || pos.x >= p.x + s.x || pos.y >= p.y + s.y || pos.z >= p.z + s.z);
    }
};

#endif // MATHS_AABB_H_
