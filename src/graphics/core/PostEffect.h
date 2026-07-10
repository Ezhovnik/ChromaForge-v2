#pragma once

#include <memory>
#include <string>
#include <variant>
#include <unordered_map>

#include <glm/glm.hpp>

#include <data/dv_fwd.h>
#include <util/EnumMetadata.h>

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

        CHROMA_ENUM_METADATA(Type)
            {"float", Type::Float},
            {"vec2", Type::Vec2},
            {"vec3", Type::Vec3},
            {"vec4", Type::Vec4},
        CHROMA_ENUM_END

        using Value = std::variant<float, glm::vec2, glm::vec3, glm::vec4>;

        Type type;
        Value defValue;
        Value value;
        bool dirty = true;

        Param();
        Param(Type type, Value defValue);
    };

    PostEffect(
        std::shared_ptr<ShaderProgram> shader,
        std::unordered_map<std::string, Param> params
    );
    explicit PostEffect(const PostEffect&) = default;

    ShaderProgram& use();

    float getIntensity() const;
    void setIntensity(float value);

    void setParam(const std::string& name, const dv::value& value);

    bool isActive() {
        return intensity > 1e-4f;
    }
private:
    std::shared_ptr<ShaderProgram> shader;
    std::unordered_map<std::string, Param> params;
    float intensity = 0.0f;
};
