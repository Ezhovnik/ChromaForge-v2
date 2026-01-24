#ifndef CODERS_PNG_H_
#define CODERS_PNG_H_

#include <string>

class Texture;
class ImageData;

// Пространство имен для операций загрузки и сохранения PNG изображений
namespace png {
    extern ImageData* loadImage(std::string filename, bool flipVertically = true); // Загружает изображение из PNG файла
    extern bool writeImage(std::string filename, const ImageData* image); // Сохраняет изображение в PNG файл
    extern Texture* loadTexture(std::string filename); // Загружает текстуру из PNG файла
}

#endif // CODERS_PNG_H_
