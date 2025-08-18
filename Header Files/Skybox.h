#ifndef SKYBOX_CLASS_H
#define SKYBOX_CLASS_H

#include "SkyboxMesh.h"

class Skybox {
    public:
        CubeTexture dayTexture;
        CubeTexture nightTexture;
        SkyboxMesh mesh;

        Skybox(std::string path);

        void setDayDuration(int dayDurationInSparks);
        void Draw(Shader& shader, Camera& camera, float timesOfDayInSparks);
    private:
        CubeTexture loadTexture (std::string path, int slot);
        int dayDurationInSparks;
};

#endif