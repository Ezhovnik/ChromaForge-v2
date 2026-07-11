#pragma once

#include <typedefs.h>

class ShadowMap {
public:
    ShadowMap(int resolution);
    ~ShadowMap();

    void bind();
    void unbind();
    uint getDepthMap() const;
    int getResolution() const;
private:
    uint fbo;
    uint depthMap;
    int resolution;
};
