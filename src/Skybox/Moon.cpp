#include "../../Header Files/Skybox/Moon.h"

Moon::Moon(glm::vec3 diskColor, glm::vec3 glowColor, float size, float glowStrength, float alpha, float orbitRadius, float Lunation) {
    Moon::diskColor = diskColor;
    Moon::glowColor = glowColor;
    Moon::size = size;
    Moon::glowStrength = glowStrength;
    Moon::alpha = alpha;
    Moon::orbitRadius = orbitRadius;
    Moon::Lunation = Lunation;

    Moon::currentAngle = 0.0f;
    Moon::phaseFactor = 0.0f;
    Moon::position = glm::vec3(0.0f, orbitRadius, 0.0f);
}

void Moon::setLunation(float Lunation) {
    Moon::Lunation = Lunation;
}

// Обновляет позицию солнца на основе времени суток
void Moon::updatePosition(float timesOfDay) {
    currentAngle = timesOfDay * 2.0f * 3.14159265359f;
    
    position.x = -sin(currentAngle) * orbitRadius;
    position.y = cos(currentAngle) * orbitRadius;
    position.z = 0.0f;
}

void Moon::updatePhase(int currDay) {
    phaseFactor = fmod((float)(currDay - 1), Lunation);
}

// Устанавливает uniform-переменные солнца в шейдере
void Moon::setShaderUniforms(Shader& shader) {
    shader.setVec3("moonDiskColor", diskColor);
    shader.setVec3("moonGlowColor", glowColor);
    shader.setFloat("moonSize", size);
    shader.setFloat("moonGlowStrength", glowStrength);
    shader.setFloat("moonAlpha", alpha);
    shader.setVec3("moonPosition", position);
    // shader.setFloat("moonPhase", phaseFactor);
}