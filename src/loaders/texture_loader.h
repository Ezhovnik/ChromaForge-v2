#ifndef LOADERS_TEXTURE_LOADER_H_
#define LOADERS_TEXTURE_LOADER_H_

#include <string>

class Texture; // Предварительное объявление класса текстур

extern Texture* loadTexture(std::string filename); // Загружает текстуру из файла и создает объект Texture

#endif // LOADERS_TEXTURE_LOADER_H_