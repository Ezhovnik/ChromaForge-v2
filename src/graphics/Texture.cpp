#include "Texture.h"

#include <GL/glew.h>

// Конструктор класса Текстур
Texture::Texture(uint id, int width, int height) : id(id), width(width), height(height){
}

Texture::Texture(ubyte* data, int width, int height) : width(width), height(height) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) data);
	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Деструктор класса Текстур
Texture::~Texture() {
    glDeleteTextures(1, &id);
}

// Активирует и привязывает текстуру для использования в отрисовке
void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::reload(ubyte* data){
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) data);
	glBindTexture(GL_TEXTURE_2D, 0);
}
