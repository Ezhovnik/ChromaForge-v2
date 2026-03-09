#include "Assets.h"

#include "../graphics/Texture.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Font.h"
#include "../graphics/Atlas.h"
#include "../logger/Logger.h"
#include "../frontend/UIDocument.h"
#include "../logic/scripting/scripting.h"

// Деструктор
Assets::~Assets() {
}

// Получает текстуру по имени
Texture* Assets::getTexture(std::string name) const {
    // Ищем текстуру в словаре
    auto it = textures.find(name);

    // Если нашли, возвращаем указатель на текстуру
    if (it != textures.end()) return it->second.get();

    // Текстура не найдена
	return nullptr;
}

// Сохраняет текстуру в менеджере ресурсов.
bool Assets::store(Texture* texture, std::string name){
    // Проверяем, не существует ли уже текстура с таким именем
    auto it = textures.find(name);

    // Если не существует, то добавляем
    if (it == textures.end()) {
        textures[name].reset(texture);
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
    if (it != shaders.end()) return it->second.get();

    // Шейдер не найден
    return nullptr;
}

// Сохраняет шейдер в менеджере ресурсов.
bool Assets::store(ShaderProgram* shader, std::string name){
    // Проверяем, не существует ли уже шейдер с таким именем
    auto it = shaders.find(name);

    // Если не существует, то добавляем
    if (it == shaders.end()) {
        shaders[name].reset(shader);
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
    if (it != fonts.end()) return it->second.get();

    // Шрифт не найден
    return nullptr;
}

// Сохраняет шрифт в менеджере ресурсов.
bool Assets::store(Font* font, std::string name){
    // Проверяем, не существует ли уже шрифт с таким именем
    auto it = fonts.find(name);

    // Если не существует, то добавляем
    if (it == fonts.end()) {
        fonts[name].reset(font);
        return true;
    }

    // Если существует, то возращаем ошибку
    LOG_WARN("Font named '{}' already exists", name);
    return false;
}

// Получает атлас по имени
Atlas* Assets::getAtlas(std::string name) const {
	// Ищем атлас в словаре
    auto it = atlases.find(name);

    // Если нашли, возвращаем указатель на атлас
    if (it != atlases.end()) return it->second.get();

    // Атлас не найден
    return nullptr;
}

// Сохраняет атлас в менеджере ресурсов
bool Assets::store(Atlas* atlas, std::string name){
	// Проверяем, не существует ли уже атлас с таким именем
    auto it = atlases.find(name);

    // Если не существует, то добавляем
    if (it == atlases.end()) {
        atlases[name].reset(atlas);
        return true;
    }

    // Если существует, то возращаем ошибку
    LOG_WARN("Atlas named '{}' already exists", name);
    return false;
}

const std::vector<TextureAnimation>& Assets::getAnimations() {
	return animations;
}

void Assets::store(const TextureAnimation& animation) {
	animations.emplace_back(animation);
}

UIDocument* Assets::getLayout(std::string name) const {
    auto it = layouts.find(name);

    if (it != layouts.end()) return it->second.get();

    return nullptr;
}

bool Assets::store(UIDocument* layout, std::string name){
    auto it = layouts.find(name);

    if (it == layouts.end()) {
        layouts[name].reset(layout);
        return true;
    }

    LOG_WARN("Layout named '{}' already exists", name);
    return false;
}

void Assets::extend(const Assets& assets) {
    for (auto entry : assets.textures) {
        textures[entry.first] = entry.second;
    }

    for (auto entry : assets.shaders) {
        shaders[entry.first] = entry.second;
    }

    for (auto entry : assets.fonts) {
        fonts[entry.first] = entry.second;
    }

    for (auto entry : assets.atlases) {
        atlases[entry.first] = entry.second;
    }

    for (auto entry : assets.layouts) {
		layouts[entry.first] = entry.second;
	}

    animations.clear();
    for (auto entry : assets.animations) {
		animations.emplace_back(entry);
	}
}
