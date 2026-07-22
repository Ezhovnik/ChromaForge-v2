#include <coders/png.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <graphics/core/ImageData.h>
#include <graphics/core/Texture.h>
#include <debug/Logger.h>
#include <typedefs.h>
#include <io/io.h>

std::unique_ptr<ImageData> png::loadImage(const ubyte* bytes, size_t size, bool flipVertically) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(flipVertically);
    stbi_uc* data = stbi_load_from_memory(bytes, static_cast<int>(size), &width, &height, &channels, 4);
    if (!data) {
        const char* error_msg = stbi_failure_reason();
        THROW_ERR("Failed to load image. Reason: {}", error_msg ? error_msg : "Unknown error");
    }

    auto imageData = std::make_unique<ubyte[]>(width * height * 4);
    std::memcpy(imageData.get(), data, width * height * 4);

    stbi_image_free(data);

    return std::make_unique<ImageData>(ImageFormat::rgba8888, width, height, std::move(imageData));
}

bool png::writeImage(const std::string& filename, const ImageData* image) {
    // Проверяем корректность входных данных
    if (!image || !image->getData()) {
        LOG_ERROR("Invalid image {} data for writing to file", filename);
        return false;
    }

    const int width = image->getWidth();
    const int height = image->getHeight();
    int channels = 0;

    // Определяем количество каналов по формату
    switch (image->getFormat()) {
        case ImageFormat::rgba8888:
            channels = 4;
            break;
        case ImageFormat::rgb888:
            channels = 3;
            break;
        default:
            LOG_ERROR("Unsupported image format for PNG writing");
            return false;
    }

    const ubyte* data = static_cast<const ubyte*>(image->getData());

    // Записываем PNG-файл с помощью stb_image_write
    int success = stbi_write_png(filename.c_str(), width, height, channels, data, width * channels);

    if (!success) {
        const char* error_msg = stbi_failure_reason();
        LOG_ERROR("Failed to write image to file: '{}'. Reason: {}", filename, error_msg ? error_msg : "Unknown error");
        return false;
    }

    return true;
}

// Загружает текстуру из PNG файла
std::unique_ptr<Texture> png::loadTexture(const ubyte* bytes, size_t size) {
    auto image = loadImage(bytes, size, true);

    // Создание объекта Texture
    auto texture = Texture::from(image.get());
    texture->setNearestFilter(); // Устанавливаем фильтрацию без сглаживания (для пиксельной графики)
    return texture;
}

std::unique_ptr<Texture> png::loadTexture(const std::string& filename) {
    auto bytes = io::read_bytes_buffer(filename);
    try {
        return loadTexture(bytes.data(), bytes.size());
    } catch (const std::runtime_error& err) {
        THROW_ERR("Could not to load '{}'", filename);
    }
}
