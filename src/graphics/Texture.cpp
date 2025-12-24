#include "Texture.h"

#include <GL/glew.h>

// Конструктор класса Текстур
Texture::Texture(uint id, int width, int height) : id(id), width(width), height(height){
}

// Деструктор класса Текстур
Texture::~Texture() {
    glDeleteTextures(1, &id);
}

// Активирует и привязывает текстуру для использования в отрисовке
void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}