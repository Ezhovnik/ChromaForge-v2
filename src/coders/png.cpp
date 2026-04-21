#include <coders/png.h>

#include <iostream>

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <graphics/core/ImageData.h>
#include <graphics/core/GLTexture.h>
#include <debug/Logger.h>
#include <typedefs.h>

std::unique_ptr<ImageData> _loadImage(const std::string& filename, bool flipVertically) {
    int channels = 0, width = 0, height = 0;

    stbi_set_flip_vertically_on_load(flipVertically); // Устанавливаем флаг вертикального переворота
    // Загружаем изображение через stb_image (возвращает указатель на данные RGB/RGBA)
    ubyte* stb_data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!stb_data) {
        const char* error_msg = stbi_failure_reason();
        LOG_ERROR("Failed to load image: '{}'. Reason: {}", filename, error_msg ? error_msg : "Unknown error");
        return nullptr;
    }

    // Определяем формат пикселей по количеству каналов
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

    // Вычисляем размер данных и копируем их в новый буфер
    const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    const size_t dataSize = pixelCount * static_cast<size_t>(channels);

    auto image_data = std::make_unique<ubyte[]>(dataSize);
    memcpy(image_data.get(), stb_data, dataSize);

    // Освобождаем память stb_image
    stbi_image_free(stb_data);

    // Создаём объект ImageData, который будет владеть скопированными данными
    auto image = std::make_unique<ImageData>(
        format, 
        width, 
        height, 
        std::move(image_data)
    );

    LOG_DEBUG("Succesfully loaded PNG image: '{}' ({}x{}, {} channels)", filename, width, height, channels);

    return image;
}

std::unique_ptr<ImageData> png::loadImage(const std::string& filename, bool flipVertically) {
    auto image = _loadImage(filename, flipVertically);
    if (image == nullptr) {
        LOG_ERROR("Could not load image '{}'", filename);
        throw std::runtime_error("Could not load image '" + filename + "'");
    }
    return image;
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
std::unique_ptr<Texture> png::loadTexture(const std::string& filename) {
    std::unique_ptr<ImageData> image(loadImage(filename, true));

    // Создание объекта Texture
    auto texture = GLTexture::from(image.get());
    texture->setNearestFilter(); // Устанавливаем фильтрацию без сглаживания (для пиксельной графики)
    return texture;
}
