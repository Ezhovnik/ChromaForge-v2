#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>

typedef unsigned int uint;

// Класс, представляющий текстуру в графической системе
class Texture {
public:
    uint id; // Идентификатор текстуры (используется в OpenGL)
    int width, height; // Ширина и высота текстуры в пикселях
    Texture (uint id, int width, int height); // Конструктор
    Texture(unsigned char* data, int width, int height);
    ~Texture(); // Деструктор

    void bind(); // Привязывает текстуру к текущему контексту OpenGL для использования
};

extern Texture* loadTexture(std::string filename); // Функция для загрузки текстуры из файла

#endif // GRAPHICS_TEXTURE_H_
