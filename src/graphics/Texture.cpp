#include "Texture.h"

#include <stdexcept>

#include <GL/glew.h>

#include "../logger/Logger.h"
#include "ImageData.h"

// Конструктор класса Текстур
Texture::Texture(uint id, int width, int height) : id(id), width(width), height(height){
}

Texture::Texture(ubyte* data, int width, int height, uint format) : width(width), height(height) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, (GLvoid *) data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);

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

Texture* Texture::from(const ImageData* image) {
	uint format;
    switch (image->getFormat())	{
		case ImageFormat::rgb888: format = GL_RGB; break;
		case ImageFormat::rgba8888: format = GL_RGBA; break;
		default:
            LOG_CRITICAL("Unsupported image data format");
			throw std::runtime_error("unsupported image data format");
	}
	const void* data = image->getData();
	return new Texture((ubyte*)data, image->getWidth(), image->getHeight(), format);
}
