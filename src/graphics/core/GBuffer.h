#pragma once

#include <typedefs.h>
#include <graphics/core/commons.h>
#include <graphics/core/ImageData.h>

class GBuffer : public Bindable {
public:
    GBuffer(uint width, uint height);
    ~GBuffer() override;

    void bind() override;
    void bindSSAO() const;
    void unbind() override;

    void bindBuffers() const;
    void bindSSAOBuffer() const;
    void bindDepthBuffer(int drawFbo);

    void resize(uint width, uint height);

    uint getWidth() const;
    uint getHeight() const;

    std::unique_ptr<ImageData> toImage() const;
private:
    uint width;
    uint height;

    uint fbo;
    uint colorBuffer = 0;
    uint positionsBuffer = 0;
    uint normalsBuffer = 0;
    uint emissionBuffer = 0;
    uint depthBuffer = 0;
    uint ssaoFbo = 0;
    uint ssaoBuffer = 0;

    void createColorBuffer();
    void createPositionsBuffer();
    void createNormalsBuffer();
    void createEmissionBuffer();
    void createDepthBuffer();
    void createSSAOBuffer();
};
