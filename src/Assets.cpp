#include "Assets.h"

#include <iostream>

#include "graphics/Texture.h"
#include "graphics/ShaderProgram.h"

// Деструктор
Assets::~Assets() {
    // Освобождаем память выделенную под шейдеры
	for (auto& iter : shaders){
		delete iter.second;
	}

    // Освобождаем память выделенную под текстуры
	for (auto& iter : textures){
		delete iter.second;
	}
}

// Получает текстуру по имени
Texture* Assets::getTexture(std::string name){
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
    std::cerr << "ERROR::Texture named '" << name << "' already exists" << std::endl;
    return false;
}

// Получает шейдер по имени
ShaderProgram* Assets::getShader(std::string name){
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
    std::cerr << "ERROR::Shader named '" << name << "' already exists" << std::endl;
    return false;
}
