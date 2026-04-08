#ifndef MATHS_AABB_H_
#define MATHS_AABB_H_

#include <glm/glm.hpp>

// Структура, представляющая выровненный по осям ограничивающий параллелепипед (AABB).
struct AABB {
	glm::vec3 a {0.0f}; // Первая точка параллелепипеда
    glm::vec3 b {1.0f, 1.0f, 1.0f}; // Вторая точка параллелепипеда

    AABB() = default;
    AABB(glm::vec3 size) : a(0.0f), b(size) {}
    AABB(glm::vec3 pos, glm::vec3 size) : a(pos), b(size) {}

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
    inline bool contains(const glm::vec3 pos) const {
        const glm::vec3 p = min();
        const glm::vec3 s = size();
        return !(pos.x < p.x || pos.y < p.y || pos.z < p.z || pos.x >= p.x + s.x || pos.y >= p.y + s.y || pos.z >= p.z + s.z);
    }

    void fix() {
        auto beg = min();
        auto end = max();
        a = beg;
        b = end;
    }

    inline void addPoint(glm::vec3 p) {
        a = glm::min(a, p);
        b = glm::max(b, p);
    }

    void transform(const glm::mat4& matrix) {
        auto pa = a;
        auto pb = b;
        a = matrix * glm::vec4(a, 1.0f);
        b = matrix * glm::vec4(b, 1.0f);
        fix();
        addPoint(matrix * glm::vec4(pb.x, pa.y, pa.z, 1.0f));
        addPoint(matrix * glm::vec4(pb.x, pb.y, pa.z, 1.0f));
        addPoint(matrix * glm::vec4(pb.x, pb.y, pb.z, 1.0f));
        addPoint(matrix * glm::vec4(pa.x, pb.y, pa.z, 1.0f));
        addPoint(matrix * glm::vec4(pa.x, pa.y, pb.z, 1.0f));
        addPoint(matrix * glm::vec4(pb.x, pa.y, pb.z, 1.0f));
    }

    inline bool intersect(const AABB& aabb) {
        return (
            a.x <= aabb.b.x &&
            b.x >= aabb.a.x && 
            a.y <= aabb.b.y &&
            b.y >= aabb.a.y && 
            a.z <= aabb.b.z &&
            b.z >= aabb.a.z 
        );
    }
};

#endif // MATHS_AABB_H_
