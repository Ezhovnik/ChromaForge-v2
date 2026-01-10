#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>

#include "../typedefs.h"

// Класс, представляющий текстуру в графической системе
class Texture {
public:
    uint id; // Идентификатор текстуры (используется в OpenGL)
    int width, height; // Ширина и высота текстуры в пикселях

    Texture (uint id, int width, int height); // Конструктор
    Texture(ubyte* data, int width, int height);
    ~Texture(); // Деструктор

    void bind(); // Привязывает текстуру к текущему контексту OpenGL для использования
    void reload(ubyte* data);
};

extern Texture* loadTexture(std::string filename); // Функция для загрузки текстуры из файла

#endif // GRAPHICS_TEXTURE_H_
