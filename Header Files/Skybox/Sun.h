#ifndef SUN_CLASS_H
#define SUN_CLASS_H

#include "../shaderClass.h"

class Sun {
    private:
        glm::vec3 position;
        glm::vec3 color;

        float size;
        float glowStrength;
        float alpha;
        
        float orbitRadius;
        float currentAngle; 
        
    public:
        Sun(glm::vec3 color = glm::vec3(1.0f, 0.98f, 0.09f), 
            float size = 0.999f, 
            float glowStrength = 0.5f, 
            float alpha = 1.0f,
            float orbitRadius = 100.0f);
        
        void updatePosition(float timesOfDay);
        void setShaderUniforms(Shader& shader);
};

#endif