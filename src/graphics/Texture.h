#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>

#include "../typedefs.h"

class ImageData;

/**
 * @brief Класс, представляющий текстуру в графической системе (OpenGL).
 *
 * Управляет созданием, загрузкой данных, настройкой параметров и освобождением
 * ресурсов текстуры. Поддерживает мип-карты, фильтрацию и чтение обратно в память.
 */
class Texture {
public:
    uint id; ///< Идентификатор текстуры в OpenGL
    int width, height; ///< Размеры текстуры в пикселях

    /**
     * @brief Конструктор, создающий объект из уже существующего OpenGL-идентификатора.
     * @param id Идентификатор существующей текстуры.
     * @param width Ширина.
     * @param height Высота.
     *
     * Используется, если текстура уже была создана ранее.
     */
    Texture (uint id, int width, int height);

    /**
     * @brief Конструктор, создающий новую текстуру из переданных пиксельных данных.
     * @param data Указатель на пиксельные данные (формат RGBA или RGB).
     * @param width Ширина изображения.
     * @param height Высота изображения.
     * @param format Формат данных OpenGL (например, GL_RGBA, GL_RGB).
     *
     * Автоматически генерирует мип-карты и устанавливает параметры фильтрации по умолчанию.
     */
    Texture(ubyte* data, int width, int height, uint format);

    /// Деструктор, удаляющий текстуру из памяти OpenGL.
    ~Texture();

    /**
     * @brief Привязывает текстуру к текущему контексту для использования в рендеринге.
     *
     * Вызывает glBindTexture(GL_TEXTURE_2D, id).
     */
    void bind();

    /**
     * @brief Перезагружает пиксельные данные текстуры (без изменения параметров).
     * @param data Новые пиксельные данные (формат должен совпадать с исходным).
     *
     * Используется для обновления содержимого существующей текстуры.
     */
    void reload(ubyte* data);

    /**
     * @brief Читает текущие пиксельные данные текстуры из видеопамяти.
     * @return Новый объект ImageData, содержащий копию пикселей в формате RGBA.
     *         Владелец должен удалить его.
     */
    ImageData* readData();

    /**
     * @brief Устанавливает фильтрацию без сглаживания (GL_NEAREST) для магнификации и минификации.
     */
    void setNearestFilter();

    /**
     * @brief Создаёт текстуру из объекта ImageData.
     * @param image Указатель на ImageData (поддерживаются форматы RGB и RGBA).
     * @return Указатель на новый экземпляр Texture.
     * @throw std::runtime_error если формат изображения не поддерживается.
     */
    static Texture* from(const ImageData* image);
};

#endif // GRAPHICS_TEXTURE_H_
