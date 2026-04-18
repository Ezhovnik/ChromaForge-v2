#include "Texture.h"

#include "GLTexture.h"

std::unique_ptr<Texture> Texture::from(const ImageData* image) {
    return GLTexture::from(image);
}
