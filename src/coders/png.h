#pragma once

#include <string>
#include <memory>

#include <typedefs.h>

class Texture;
class ImageData;

/**
 * @brief Пространство имён для операций загрузки и сохранения PNG изображений.
 *
 * Использует библиотеку stb_image для чтения и записи PNG.
 * Поддерживаются форматы RGB (3 канала) и RGBA (4 канала).
 */
namespace png {
    std::unique_ptr<ImageData> loadImage(const ubyte* bytes, size_t size, bool flipVertically);

    /**
     * @brief Сохраняет изображение в PNG-файл.
     * @param filename Имя выходного файла.
     * @param image Указатель на объект ImageData.
     * @return true при успешной записи, false при ошибке.
     */
    bool writeImage(const std::string& filename, const ImageData* image);

    /**
     * @brief Загружает текстуру из PNG-файла.
     * @param filename Путь к файлу.
     * @return Указатель на объект Texture или nullptr в случае ошибки.
     *         Текстура создаётся с флагом GL_NEAREST фильтрации.
     */
    std::unique_ptr<Texture> loadTexture(const std::string& filename);

    std::unique_ptr<Texture> loadTexture(const ubyte* bytes, size_t size);
}
