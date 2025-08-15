#include "../Header Files/CubeTexture.h"

CubeTexture::CubeTexture(std::vector<std::string> faces, const char* texType, GLuint slot) {
    type = texType;

    // Создаем текстурный объект OpenGL
    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    unit = slot;
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

    // Загружаем каждое изображение для кубической текстуры
    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* bytes = stbi_load(faces[i].c_str(), &widthImg, &heightImg, &numColCh, 0);
        if (!bytes) {
            throw std::runtime_error(("Failed to load texture: " + std::string(faces[i])).c_str());
        }

        GLenum format;
        if (numColCh == 4)
            format = GL_RGBA;
        else if (numColCh == 3)
            format = GL_RGB;
        else if (numColCh == 1)
            format = GL_RED;
        else {
            stbi_image_free(bytes);
            throw std::invalid_argument("Automatic Texture type recognition failed");
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, format, widthImg, heightImg, 0, format, GL_UNSIGNED_BYTE, bytes
        );
        stbi_image_free(bytes);
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Устанавливаем параметры текстуры
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,  GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Отменяем привязку
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeTexture::texUnit(Shader& shader, const char* uniform, GLuint unit) {
    GLuint texUni = glGetUniformLocation(shader.ID, uniform);
    shader.Activate();
    glUniform1i(texUni, unit);
}

void CubeTexture::Bind() {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void CubeTexture::Unbind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeTexture::Delete() {
    glDeleteTextures(1, &ID);
}