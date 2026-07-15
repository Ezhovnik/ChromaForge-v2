#pragma once

#include <vector>
#include <memory>
#include <string>
#include <variant>
#include <unordered_map>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <data/dv_fwd.h>
#include <util/EnumMetadata.h>

class ShaderProgram;

class PostEffect {
public:
    struct Param {
        enum class Type {
            Int,
            Float,
            Vec2,
            Vec3,
            Vec4
        };

        CHROMA_ENUM_METADATA(Type)
            {"int", Type::Int},
            {"float", Type::Float},
            {"vec2", Type::Vec2},
            {"vec3", Type::Vec3},
            {"vec4", Type::Vec4},
        CHROMA_ENUM_END

        using Value = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4>;

        Type type;
        Value defValue;
        Value value;
        bool array = false;
        bool dirty = true;

        Param();
        Param(Type type, Value defValue, bool array);
    };

    PostEffect(
        bool advanced,
        std::shared_ptr<ShaderProgram> shader,
        std::unordered_map<std::string, Param> params
    );
    explicit PostEffect(const PostEffect&) = default;

    ShaderProgram& use();
    ShaderProgram& getShader();

    float getIntensity() const;
    void setIntensity(float value);

    void setParam(const std::string& name, const dv::value& value);
    void setArray(const std::string& name, std::vector<ubyte>&& values);

    bool isAdvanced() const {
        return advanced;
    }

    bool isActive() {
        return intensity > 1e-4f;
    }
private:
    bool advanced = false;
    std::shared_ptr<ShaderProgram> shader;
    std::unordered_map<std::string, Param> params;
    std::unordered_map<std::string, std::vector<ubyte>> arrayValues;
    float intensity = 0.0f;
};
