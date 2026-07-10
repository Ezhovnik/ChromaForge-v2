#include <graphics/core/PostEffect.h>

#include <graphics/core/ShaderProgram.h>
#include <data/dv_util.h>

PostEffect::Param::Param() : type(Type::Float) {}

PostEffect::Param::Param(
    Type type,
    Value defValue
) : type(type),
    defValue(defValue),
    value(defValue) {}

PostEffect::PostEffect(
    std::shared_ptr<ShaderProgram> shader,
    std::unordered_map<std::string, Param> params
) : shader(std::move(shader)),
    params(std::move(params)) {}

ShaderProgram& PostEffect::use() {
    shader->use();
    shader->uniform1f("u_intensity", intensity);
    for (auto& [name, param] : params) {
        if (!param.dirty) continue;

        switch (param.type) {
            case Param::Type::Float:
                shader->uniform1f(name, std::get<float>(param.value));
                break;
            case Param::Type::Vec2:
                shader->uniform2f(name, std::get<glm::vec2>(param.value));
                break;
            case Param::Type::Vec3:
                shader->uniform3f(name, std::get<glm::vec3>(param.value));
                break;
            case Param::Type::Vec4:
                shader->uniform4f(name, std::get<glm::vec4>(param.value));
                break;
            default:
                assert(false);
        }
        param.dirty = false;
    }
    return *shader;
}

float PostEffect::getIntensity() const {
    return intensity;
}

void PostEffect::setIntensity(float value) {
    intensity = value;
}

template<int n>
static void set_value(PostEffect::Param::Value& dst, const dv::value& value) {
    glm::vec<n, float> vec;
    dv::get_vec(value, vec);
    dst = vec;
}

void PostEffect::setParam(const std::string& name, const dv::value& value) {
    const auto& found = params.find(name);
    if (found == params.end()) {
        return;
    }
    auto& param = found->second;
    switch (param.type) {
        case Param::Type::Float:
            param.value = static_cast<float>(value.asNumber());
            break;
        case Param::Type::Vec2:
            set_value<2>(param.value, value);
            break;
        case Param::Type::Vec3:
            set_value<3>(param.value, value);
            break;
        case Param::Type::Vec4:
            set_value<4>(param.value, value);
            break;
    }
    param.dirty = true;
}
