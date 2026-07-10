#pragma once

#include <memory>
#include <string>
#include <variant>
#include <unordered_map>

#include <glm/glm.hpp>

class ShaderProgram;

class PostEffect {
public:
    struct Param {
        enum class Type {
            Float,
            Vec2,
            Vec3,
            Vec4
        };
        using Value = std::variant<float, glm::vec2, glm::vec3, glm::vec4>;

        Type type;
        Value defValue;

        Param();
        Param(Type type, Value defValue);
    };

    PostEffect(
        std::unique_ptr<ShaderProgram> shader,
        std::unordered_map<std::string, Param> params
    );

    ShaderProgram& use();

    void setIntensity(float value);

    bool isActive() {
        return intensity > 1e-4f;
    }
private:
    std::unique_ptr<ShaderProgram> shader;
    std::unordered_map<std::string, Param> params;
    float intensity = 0.0f;
};
