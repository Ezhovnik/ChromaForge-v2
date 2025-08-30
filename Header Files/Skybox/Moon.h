#ifndef MOON_CLASS_H
#define MOON_CLASS_H

#include "../shaderClass.h"

class Moon {
    private:
        glm::vec3 position;
        glm::vec3 diskColor;
        glm::vec3 glowColor;

        float size;
        float glowStrength;
        float alpha;
        float phaseFactor;
        int Lunation;
        
        float orbitRadius;
        float currentAngle; 
        
    public:
        Moon(glm::vec3 diskColor = glm::vec3(0.49f, 0.46f, 0.47f),
            glm::vec3 glowColor = glm::vec3(0.7f, 0.82f, 0.89f),
            float size = 0.9995f, 
            float glowStrength = 0.25f, 
            float alpha = 1.0f,
            float orbitRadius = 100.0f,
            float Lunation = 29.53
        );
        
        void setLunation(float Lunation);
        void updatePosition(float timesOfDay);
        void updatePhase(int currDay);
        void setShaderUniforms(Shader& shader);
};

#endif