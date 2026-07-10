#include <graphics/core/PostEffect.h>

#include <graphics/core/ShaderProgram.h>

PostEffect::Param::Param() : type(Type::Float) {}

PostEffect::Param::Param(Type type, Value defValue) : type(type), defValue(std::move(defValue)) {
}

PostEffect::PostEffect(
    std::unique_ptr<ShaderProgram> shader,
    std::unordered_map<std::string, Param> params
) : shader(std::move(shader)),
    params(std::move(params)) {}

ShaderProgram& PostEffect::use() {
    shader->use();
    shader->uniform1f("u_intensity", intensity);
    return *shader;
}

void PostEffect::setIntensity(float value) {
    intensity = value;
}
