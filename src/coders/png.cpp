#include "png.h"

#include <iostream>

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"

#include "../graphics/ImageData.h"
#include "../graphics/Texture.h"
#include "../logger/Logger.h"
#include "../typedefs.h"

ImageData* png::loadImage(std::string filename) {
    int channels, width, height;

    stbi_set_flip_vertically_on_load(true);
    ubyte* stb_data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    
    if (!stb_data) {
        LOG_ERROR("Failed to load image: '{}'\n{}", filename, stbi_failure_reason());
        return nullptr;
    }
    
    ImageFormat format;
    switch (channels) {
        case 4:
            format = ImageFormat::rgba8888;
            break;
        case 3:
            format = ImageFormat::rgb888;
            break;
        default:
            LOG_ERROR("Unsupported number of channels: {}", channels);
            stbi_image_free(stb_data);
            return nullptr;
    }

    size_t data_size = width * height * channels;
    ubyte* image_data = new ubyte[data_size];
    memcpy(image_data, stb_data, data_size);

    stbi_image_free(stb_data);

    ImageData* image = new ImageData(format, width, height, image_data);

    return image;
}

// Функция загрузки текстуры
Texture* png::loadTexture(std::string filename) {
    ImageData* image = loadImage(filename);

    if (image == nullptr) {
        LOG_CRITICAL("Could not load texture '{}'", filename);
        return nullptr;
    }

    return Texture::from(image); // Создание объекта Texture
}
