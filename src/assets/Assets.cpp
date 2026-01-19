#include "Assets.h"

#include <iostream>

#include "../graphics/Texture.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Font.h"
#include "../logger/Logger.h"

// Деструктор
Assets::~Assets() {
    // Освобождаем память выделенную под шейдеры
	for (auto& iter : shaders){
		if (iter.second != nullptr) {
            delete iter.second;
            iter.second = nullptr;
        }
	}
    shaders.clear();

    // Освобождаем память выделенную под текстуры
	for (auto& iter : textures){
		if (iter.second != nullptr) {
            delete iter.second;
            iter.second = nullptr;
        }
	}
    textures.clear();

    // Освобождаем память выделенную под шрифты
    for (auto& iter : fonts){
		if (iter.second != nullptr) {
            delete iter.second;
            iter.second = nullptr;
        }
	}
    fonts.clear();
}

// Получает текстуру по имени
Texture* Assets::getTexture(std::string name) const{
    // Ищем текстуру в словаре
    auto it = textures.find(name);

    // Если нашли, возвращаем указатель на текстуру
    if (it != textures.end()) return it->second;

    // Текстура не найдена
	return nullptr;
}

// Сохраняет текстуру в менеджере ресурсов.
bool Assets::store(Texture* texture, std::string name){
    // Проверяем, не существует ли уже текстура с таким именем
    auto it = textures.find(name);

    // Если не существует, то добавляем
    if (it == textures.end()) {
        textures[name] = texture;
        return true;
    }

    // Если существует, то возращаем ошибку
    LOG_WARN("Texture named '{}' already exists", name);
    return false;
}

// Получает шейдер по имени
ShaderProgram* Assets::getShader(std::string name) const{
    // Ищем шейдер в словаре
    auto it = shaders.find(name);

    // Если нашли, возвращаем указатель на шейдер
    if (it != shaders.end()) return it->second;

    // Шейдер не найден
    return nullptr;
}

// Сохраняет шейдер в менеджере ресурсов.
bool Assets::store(ShaderProgram* shader, std::string name){
    // Проверяем, не существует ли уже шейдер с таким именем
    auto it = shaders.find(name);

    // Если не существует, то добавляем
    if (it == shaders.end()) {
        shaders[name] = shader;
        return true;
    }

    // Если существует, то возращаем ошибку
    LOG_WARN("Shader named '{}' already exists", name);
    return false;
}

// Получает шрифт по имени
Font* Assets::getFont(std::string name) const{
    // Ищем шрифт в словаре
    auto it = fonts.find(name);

    // Если нашли, возвращаем указатель на шрифт
    if (it != fonts.end()) return it->second;

    // Шрифт не найден
    return nullptr;
}

// Сохраняет шрифт в менеджере ресурсов.
bool Assets::store(Font* font, std::string name){
    // Проверяем, не существует ли уже шрифт с таким именем
    auto it = fonts.find(name);

    // Если не существует, то добавляем
    if (it == fonts.end()) {
        fonts[name] = font;
        return true;
    }

    // Если существует, то возращаем ошибку
    LOG_WARN("Font named '{}' already exists", name);
    return false;
}
