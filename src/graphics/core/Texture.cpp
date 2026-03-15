#include "Texture.h"

#include <memory>

#include <GL/glew.h>

#include "../../logger/Logger.h"
#include "gl_util.h"

Texture::Texture(uint id, uint width, uint height) : id(id), width(width), height(height){
}

Texture::Texture(ubyte* data, uint width, uint height, ImageFormat imageFormat) : width(width), height(height) {
    // Генерируем уникальный идентификатор текстуры
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Загружаем данные в текстуру
    GLenum format = gl::to_gl_format(imageFormat);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, (GLvoid *) data);

    // Устанавливаем параметры фильтрации: при уменьшении используется мип-линейный, при увеличении — ближайший
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Режим повторения текстуры за пределами [0,1]
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Генерируем мип-карты
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1); // Ограничиваем максимальный уровень мип-карт

    // Отвязываем текстуру, чтобы избежать случайных изменений
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::reload(ubyte* data) {
    // Привязываем и заменяем данные без изменения параметров
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

ImageData* Texture::readData() {
    // Выделяем буфер для пикселей (RGBA, 4 байта на пиксель)
    std::unique_ptr<ubyte[]> data (new ubyte[width * height * 4]);
    glBindTexture(GL_TEXTURE_2D, id);
    // Читаем текущее содержимое текстуры в буфер
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    glBindTexture(GL_TEXTURE_2D, 0);
    return new ImageData(ImageFormat::rgba8888, width, height, data.release());
}

void Texture::setNearestFilter() {
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture* Texture::from(const ImageData* image) {
	const void* data = image->getData();
	return new Texture((ubyte*)data, image->getWidth(), image->getHeight(), image->getFormat());
}

uint Texture::getWidth() const {
    return width;
}

uint Texture::getHeight() const {
    return height;
}

uint Texture::getId() const {
    return id;
}
