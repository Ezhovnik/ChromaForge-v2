#include "../Header Files/Texture.h"

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
{
	// Назначает тип текстуры объекту texture
	type = texType;

	// Переменные для хранения ширины, высоты и количества цветовых каналов изображения
	int widthImg, heightImg, numColCh;
	// Переворачиваем изображение так, чтобы оно отображалось правой стороной вверх
	stbi_set_flip_vertically_on_load(true);
	// Считываем изображение из файла и сохраняем его в байтах
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

	// Создает текстурный объект OpenGL
	glGenTextures(1, &ID);
	// Привязываем текстуру текстурному элементу
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	// Настраивает тип алгоритма, который используется для уменьшения или увеличения размера изображения
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Настраиваем способ повторения текстуры (если это вообще происходит)
	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Дополнительные строки на случай, если решим использовать GL_CLAMP_TO_BORDER
	// float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	// Присваиваем изображение текстурному объекту OpenGL
	glTexImage2D(texType, 0, format, widthImg, heightImg, 0, format, pixelType, bytes);
	// Генерируем MIP-карты
	glGenerateMipmap(texType);

	// Удаляем данные изображения в том виде, в каком они уже есть в объекте текстуры OpenGL
	stbi_image_free(bytes);

	// Отменяет привязку объекта текстуры OpenGL, чтобы его нельзя было случайно изменить
	glBindTexture(texType, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Определяет местоположение униформы
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	// Шейдер необходимо активировать перед изменением значения униформы
	shader.Activate();
	// Устанавливает значение униформы
	glUniform1i(texUni, unit);
}

void Texture::Bind()
{
	glBindTexture(type, ID);
}

void Texture::Unbind()
{
	glBindTexture(type, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}