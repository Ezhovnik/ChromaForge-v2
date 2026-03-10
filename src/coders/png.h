#ifndef CODERS_PNG_H_
#define CODERS_PNG_H_

#include <string>

class Texture;
class ImageData;

/**
 * @brief Пространство имён для операций загрузки и сохранения PNG изображений.
 *
 * Использует библиотеку stb_image для чтения и записи PNG.
 * Поддерживаются форматы RGB (3 канала) и RGBA (4 канала).
 */
namespace png {
    /**
     * @brief Загружает изображение из PNG-файла.
     * @param filename Путь к файлу.
     * @param flipVertically Если true, изображение будет перевёрнуто по вертикали.
     * @return Указатель на объект ImageData или nullptr в случае ошибки.
     *         Владельцем объекта становится вызывающий код, он должен быть удалён.
     */
    extern ImageData* loadImage(const std::string& filename, bool flipVertically = true);

    /**
     * @brief Сохраняет изображение в PNG-файл.
     * @param filename Имя выходного файла.
     * @param image Указатель на объект ImageData.
     * @return true при успешной записи, false при ошибке.
     */
    extern bool writeImage(const std::string& filename, const ImageData* image);

    /**
     * @brief Загружает текстуру из PNG-файла.
     * @param filename Путь к файлу.
     * @return Указатель на объект Texture или nullptr в случае ошибки.
     *         Текстура создаётся с флагом GL_NEAREST фильтрации.
     *         Владельцем объекта становится вызывающий код.
     */
    extern Texture* loadTexture(const std::string& filename);
}

#endif // CODERS_PNG_H_
