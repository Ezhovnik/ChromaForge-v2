#include "Assets.h"

#include "../graphics/Texture.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Font.h"
#include "../graphics/Atlas.h"
#include "../logger/Logger.h"
#include "../frontend/UIDocument.h"
#include "../logic/scripting/scripting.h"

Assets::~Assets() {
}

Texture* Assets::getTexture(std::string name) const {
    // Ищем текстуру в словаре
    auto it = textures.find(name);

    // Если нашли, возвращаем указатель на текстуру
    if (it != textures.end()) return it->second.get();

    // Текстура не найдена
	return nullptr;
}

void Assets::store(Texture* texture, std::string name){
    textures[name].reset(texture);
}

ShaderProgram* Assets::getShader(std::string name) const{
    // Ищем шейдер в словаре
    auto it = shaders.find(name);

    // Если нашли, возвращаем указатель на шейдер
    if (it != shaders.end()) return it->second.get();

    // Шейдер не найден
    return nullptr;
}

void Assets::store(ShaderProgram* shader, std::string name){
    shaders[name].reset(shader);
}

Font* Assets::getFont(std::string name) const{
    // Ищем шрифт в словаре
    auto it = fonts.find(name);

    // Если нашли, возвращаем указатель на шрифт
    if (it != fonts.end()) return it->second.get();

    // Шрифт не найден
    return nullptr;
}

void Assets::store(Font* font, std::string name){
    fonts[name].reset(font);
}

Atlas* Assets::getAtlas(std::string name) const {
	// Ищем атлас в словаре
    auto it = atlases.find(name);

    // Если нашли, возвращаем указатель на атлас
    if (it != atlases.end()) return it->second.get();

    // Атлас не найден
    return nullptr;
}

void Assets::store(Atlas* atlas, std::string name){
    atlases[name].reset(atlas);
}

const std::vector<TextureAnimation>& Assets::getAnimations() {
	return animations;
}

void Assets::store(const TextureAnimation& animation) {
	animations.emplace_back(animation);
}

UIDocument* Assets::getLayout(std::string name) const {
    // Ищем макет в словаре
    auto it = layouts.find(name);

    // Если нашли, возвращаем указатель на макет
    if (it != layouts.end()) return it->second.get();

    // Макет не найден
    return nullptr;
}

void Assets::store(UIDocument* layout, std::string name){
    layouts[name].reset(layout);
}

void Assets::extend(const Assets& assets) {
    // Копируем текстуры
    for (auto entry : assets.textures) {
        textures[entry.first] = entry.second;
    }

    // Копируем шейдеры
    for (auto entry : assets.shaders) {
        shaders[entry.first] = entry.second;
    }

    // Копируем шрифты
    for (auto entry : assets.fonts) {
        fonts[entry.first] = entry.second;
    }

    // Копируем атласы
    for (auto entry : assets.atlases) {
        atlases[entry.first] = entry.second;
    }

    // Копируем макеты
    for (auto entry : assets.layouts) {
		layouts[entry.first] = entry.second;
	}

    // Перезаписываем анимации
    animations.clear();
    for (auto entry : assets.animations) {
		animations.emplace_back(entry);
	}
}
