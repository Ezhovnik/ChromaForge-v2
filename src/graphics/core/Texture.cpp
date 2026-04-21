#include <graphics/core/Texture.h>

#include <graphics/core/GLTexture.h>

std::unique_ptr<Texture> Texture::from(const ImageData* image) {
    return GLTexture::from(image);
}
