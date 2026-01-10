#include "texture_loader.h"

#include <iostream>

#include <GL/glew.h>
#include "../include/stb/stb_image.h"

#include "../graphics/Texture.h"
#include "../logger/Logger.h"
#include "../typedefs.h"

// Функция для загрузки изображения и создания OpenGL текстуры
GLuint loadImage(const char* file, int* width, int* height) {
    int numColCh;
    GLuint texture;

    stbi_set_flip_vertically_on_load(true); // Инвертируем изображение по вертикали (OpenGL координаты начинаются снизу)
    ubyte* bytes = stbi_load(file, width, height, &numColCh, 0); // Загружка изображения из файла

    // Проверка успешности загрузки изображения
    if (bytes == nullptr) {
        LOG_ERROR("Failed to load image: '{}'\n{}", file, stbi_failure_reason());
        return 0;
    }

    glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

    // Выбор формата текстуры в зависимости от количества каналов
    if (numColCh == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes); // RGBA формат
    } else if (numColCh == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes); // RGB формат
    } else if (numColCh == 1) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RED, GL_UNSIGNED_BYTE, bytes); // Одноканальный формат
    } else {
        LOG_ERROR("Automatic Texture type recognition failed for: '{}'\nNumber of channels: {}", file, numColCh);
        stbi_image_free(bytes);
        glDeleteTextures(1, &texture);
        return 0;
    }

    // Настройка параметров фильтрации текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // Минификация
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Магнификация
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(bytes);

    return texture;
}

// Функция загрузки текстуры
Texture* loadTexture(std::string filename) {
    int width, height;
    GLuint texture = loadImage(filename.c_str(), &width, &height);

    // Проверка успешности создания OpenGL текстуры
    if (texture == 0) {
        LOG_CRITICAL("Could not load texture '{}'", filename);
        return nullptr;
    }

    return new Texture(texture, width, height); // Создание объекта Texture
}
