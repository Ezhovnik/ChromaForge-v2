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

    void resize(uint width, uint height);

    uint getWidth() const;
    uint getHeight() const;

    std::unique_ptr<ImageData> toImage() const;
private:
    uint width;
    uint height;

    uint fbo;
    uint colorBuffer;
    uint positionsBuffer;
    uint normalsBuffer;
    uint depthBuffer;
    uint ssaoFbo;
    uint ssaoBuffer;

    void createColorBuffer();
    void createPositionsBuffer();
    void createNormalsBuffer();
    void createDepthBuffer();
    void createSSAOBuffer();
};
