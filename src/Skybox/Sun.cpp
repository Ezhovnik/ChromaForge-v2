#include "../../Header Files/Skybox/Sun.h"

Sun::Sun(glm::vec3 color, float size, float glowStrength, float alpha, float orbitRadius) {
    Sun::color = color;
    Sun::size = size;
    Sun::glowStrength = glowStrength;
    Sun::alpha = alpha;
    Sun::orbitRadius = orbitRadius;
    Sun::currentAngle = 0.0f;

    Sun::position = glm::vec3(0.0f, -orbitRadius, 0.0f);
}

// Обновляет позицию солнца на основе времени суток
void Sun::updatePosition(float timesOfDay) {
    currentAngle = timesOfDay * 2.0f * 3.14159265359f;
    
    position.x = sin(currentAngle) * orbitRadius;
    position.y = -cos(currentAngle) * orbitRadius;
    position.z = 0.0f;
}

// Устанавливает uniform-переменные солнца в шейдере
void Sun::setShaderUniforms(Shader& shader) {
    shader.setVec3("sunColor", color);
    shader.setFloat("sunSize", size);
    shader.setFloat("sunGlowStrength", glowStrength);
    shader.setFloat("sunAlpha", alpha);
    shader.setVec3("sunPosition", position);
}